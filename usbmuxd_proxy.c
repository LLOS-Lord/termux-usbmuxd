#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

/**
 * usbmuxd_proxy: Wrapper duy trì File Descriptor USB
 * Được thiết kế để bỏ qua các tín hiệu terminal và giữ usbmuxd sống bền bỉ.
 */

int main(int argc, char *argv[]) {
    // Bỏ qua các tín hiệu terminal để tránh bị kill khi shell đóng hoặc chuyển background
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    char *fd_str = getenv("FD");
    if (!fd_str) fd_str = getenv("TERMUX_USB_FD");

    if (!fd_str) {
        fprintf(stderr, "usbmuxd_proxy: ERROR: No USB FD found. Run via termux-usb!\n");
        return 1;
    }

    int fd = atoi(fd_str);
    fprintf(stderr, "usbmuxd_proxy: Maintaining USB FD: %d\n", fd);

    // Truyền LIBUSB_FD cho usbmuxd (Cực kỳ quan trọng)
    char libusb_fd_env[64];
    snprintf(libusb_fd_env, sizeof(libusb_fd_env), "LIBUSB_FD=%d", fd);
    putenv(libusb_fd_env);

    const char *usbmuxd_path = "/data/data/com.termux/files/usr/bin/usbmuxd";

    // Phân tích các đối số từ script
    int has_f = 0;
    char *unix_socket_path = NULL;
    char *tcp_port = NULL;
    char *pidfile_path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--foreground") == 0) {
            has_f = 1;
        } else if (strcmp(argv[i], "--socket") == 0 && (i + 1) < argc) {
            unix_socket_path = argv[++i];
        } else if (strcmp(argv[i], "--tcp-port") == 0 && (i + 1) < argc) {
            tcp_port = argv[++i];
        } else if (strcmp(argv[i], "--pidfile") == 0 && (i + 1) < argc) {
            pidfile_path = argv[++i];
        }
    }

    // Vì usbmuxd chỉ hỗ trợ một tùy chọn -S (socket), chúng ta sẽ ưu tiên Unix socket 
    // vì đó là mặc định của libimobiledevice. Tuy nhiên, usbmuxd trên Termux có thể 
    // không hỗ trợ chạy cả hai cùng lúc qua tham số dòng lệnh nếu chỉ có một cờ -S.
    // Theo log của người dùng: -S ADDR:PORT | PATH
    
    // Để giải quyết triệt để, chúng ta sẽ chạy usbmuxd với Unix socket mặc định 
    // và sử dụng một cơ chế proxy khác nếu cần TCP. Nhưng ở đây, yêu cầu là chạy cả hai.
    // Thử nghiệm: usbmuxd có thể không cho phép nhiều -S. 
    // Nếu vậy, chúng ta sẽ ưu tiên cái người dùng cần nhất hoặc cái ổn định nhất.
    // Tuy nhiên, để "chạy song song" như yêu cầu, tôi sẽ giả định usbmuxd có thể chạy 
    // nhiều instance hoặc hỗ trợ dual qua cấu hình. Nhưng usbmuxd thường chỉ 1 instance.
    
    // Giải pháp triệt để: usbmuxd lắng nghe trên Unix socket (mặc định) 
    // và chúng ta sẽ dùng `socat` để forward từ TCP sang Unix socket đó.
    // Điều này đảm bảo cả hai đều hoạt động mà không cần can thiệp sâu vào usbmuxd.

    int usbmuxd_argc = 1; // usbmuxd_path
    if (!has_f) usbmuxd_argc++;
    if (unix_socket_path) usbmuxd_argc += 2; // -S <path>
    if (pidfile_path) usbmuxd_argc += 2; // -P <path>
    
    char **usbmuxd_argv = malloc(sizeof(char *) * (usbmuxd_argc + 1));
    int j = 0;
    usbmuxd_argv[j++] = (char *)usbmuxd_path;
    if (!has_f) usbmuxd_argv[j++] = "-f";
    if (unix_socket_path) {
        usbmuxd_argv[j++] = "-S";
        usbmuxd_argv[j++] = unix_socket_path;
    }
    if (pidfile_path) {
        usbmuxd_argv[j++] = "-P";
        usbmuxd_argv[j++] = pidfile_path;
    }
    usbmuxd_argv[j] = NULL;

    fprintf(stderr, "usbmuxd_proxy: Starting usbmuxd...\n");
    pid_t pid = fork();

    if (pid == 0) {
        execv(usbmuxd_path, usbmuxd_argv);
        perror("usbmuxd_proxy: usbmuxd execv failed");
        exit(1);
    } else if (pid > 0) {
        // Khởi động socat để forward TCP sang Unix socket nếu có tcp_port
        if (tcp_port && unix_socket_path) {
            fprintf(stderr, "usbmuxd_proxy: Starting TCP forwarder on port %s -> %s\n", tcp_port, unix_socket_path);
            pid_t socat_pid = fork();
            if (socat_pid == 0) {
                // Chờ usbmuxd tạo socket
                sleep(2);
                char tcp_addr[64];
                snprintf(tcp_addr, sizeof(tcp_addr), "TCP4-LISTEN:%s,reuseaddr,fork", tcp_port);
                char unix_addr[256];
                snprintf(unix_addr, sizeof(unix_addr), "UNIX-CONNECT:%s", unix_socket_path);
                
                execlp("socat", "socat", tcp_addr, unix_addr, NULL);
                // Nếu không có socat, thử dùng bản build-in hoặc báo lỗi
                perror("usbmuxd_proxy: socat exec failed (TCP forwarding will not work)");
                exit(1);
            }
        }

        int status;
        waitpid(pid, &status, 0);
        fprintf(stderr, "usbmuxd_proxy: usbmuxd exited with %d.\n", status);
        // Kill socat if it's still running (tùy chọn, socat thường tự thoát nếu socket mất)
    } else {
        perror("usbmuxd_proxy: fork failed");
        return 1;
    }

    return 0;
}
