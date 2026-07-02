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

    // Ép buộc foreground (-f) để proxy có thể quản lý tiến trình con
    int has_f = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--foreground") == 0) {
            has_f = 1;
            break;
        }
    }

    int new_argc = argc + (has_f ? 0 : 1);
    char **new_argv = malloc(sizeof(char *) * (new_argc + 1));
    new_argv[0] = (char *)usbmuxd_path;
    int j = 1;
    for (int i = 1; i < argc; i++) {
        new_argv[j++] = argv[i];
    }
    if (!has_f) {
        new_argv[j++] = "-f";
    }
    new_argv[new_argc] = NULL;

    pid_t pid = fork();

    if (pid == 0) {
        // Tiến trình con: Chạy usbmuxd
        if (execv(usbmuxd_path, new_argv) == -1) {
            perror("usbmuxd_proxy: execv failed");
            exit(1);
        }
    } else if (pid > 0) {
        // Tiến trình cha: Đợi con để giữ FD luôn mở cho Android
        fprintf(stderr, "usbmuxd_proxy: usbmuxd started (PID: %d). Keeping USB connection alive...\n", pid);
        int status;
        waitpid(pid, &status, 0);
        fprintf(stderr, "usbmuxd_proxy: usbmuxd exited with %d.\n", status);
    } else {
        perror("usbmuxd_proxy: fork failed");
        return 1;
    }

    return 0;
}
