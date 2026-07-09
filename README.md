# termux-usbmuxd

## Rootless usbmuxd management for Termux 

This project provides a robust solution for running `usbmuxd` on Termux without root access, enabling various `libimobiledevice` tools to interact with iOS devices. This version includes enhancements for better stability, compatibility, and automatic support for both Unix and TCP sockets.

## Features

- **Rootless Operation**: Run `usbmuxd` and `libimobiledevice` tools without requiring root privileges on your Android device.
- **USB-OTG Support**: Seamlessly connect your iOS device via USB-OTG.
- **Stable & Persistent**: Designed to maintain `usbmuxd` daemon even when Termux is in the background or shell sessions are closed.
- **Automatic Dual-Socket Support**: `usbmuxd` will now automatically attempt to open both a Unix domain socket and a TCP socket (on port 27015 by default). This ensures maximum compatibility with various `libimobiledevice` tools, as some prefer Unix sockets while others might work better with TCP sockets in certain Termux environments.
- **Improved Error Handling**: Better logging and error messages for easier troubleshooting.

## Prerequisites

- Termux installed on your Android device.
- `termux-api` package installed in Termux (`pkg install termux-api`).
- `termux-usb` command working correctly (grant USB permission to Termux).
- An iOS device and a USB-OTG cable.

## Installation

1.  **Clone the repository and setup enviroment:**

    ```bash
    git clone https://github.com/LLOS-Lord/termux-usbmuxd.git
    cd termux-usbmuxd
    bash install.sh
    ```

    This script will:
    - Update and upgrade Termux packages.
    - Install necessary dependencies (`usbmuxd`, `libimobiledevice`, `build-essential`, `clang`, `python`, `jq`, `libzip`).
    - Compile `usbmuxd_proxy`.
    - Copy `termux-usbmuxd` script to `$PREFIX/bin`.
    - Add `USBMUXD_SOCKET_ADDRESS` environment variable to your `~/.bashrc`.

3.  **Source your `.bashrc`:**

    ```bash
    source ~/.bashrc
    ```

## Usage
Command to run Termux-usbmuxd
```bash
termux-usbmuxd
```
After run command you will see a menu function termux-usbmuxd
### Quick Start

1.  Connect your iPhone via USB-OTG cable to your Android device.
2.  Run `termux-usbmuxd` in Termux.
3.  (First time) Grant USB permission in the Android popup.
4.  Run `idevicepair pair` to pair your device.

### Commands

-   **`termux-usbmuxd`**: Starts the `usbmuxd` daemon. It will attempt to open both a Unix domain socket and a TCP socket. If no device is found, it will prompt you to connect one.
-   **`termux-usbmuxd stop`**: Stops the running `usbmuxd` daemon.
-   **`termux-usbmuxd status`**: Shows the current status of `usbmuxd`, indicating if Unix and/or TCP sockets are active.
-   **`termux-usbmuxd pair`**: Initiates the pairing process with your iOS device (uses `idevicepair pair`).
-   **`termux-usbmuxd info`**: Displays information about your connected iOS device (uses `ideviceinfo`).
-   **`termux-usbmuxd doctor`**: Checks `~/.bashrc`/`~/.zshrc` for a stale or incompatible `USBMUXD_SOCKET_ADDRESS` (e.g. an old `UNIX:/path` value) and rewrites it to the correct `127.0.0.1:27015` TCP form. Also runs automatically, silently, every time `termux-usbmuxd` starts.

## Troubleshooting

-   **"iPhone not found. Please plug in OTG cable."**: Ensure your iOS device is connected via USB-OTG and recognized by Android. Check if `termux-usb -l` lists your device.
-   **"Permission denied."**: Make sure you grant USB permission to Termux when prompted by Android.
-   **"Startup failed. Please check the log: cat ~/.termux-usbmuxd.log"**: Examine the log file for detailed error messages. This log file is crucial for diagnosing issues.
-   **Socket errors with `idevice` tools**: With the new dual-socket support, most tools should now connect automatically. If you still encounter issues, ensure `USBMUXD_SOCKET_ADDRESS` is correctly set in your `~/.bashrc` (it should point to the Unix socket by default, but tools can still connect to the TCP port).
-   **`idevicepair pair` fails**: Ensure `usbmuxd` is running (`termux-usbmuxd status` should show RUNNING for at least one socket). Try restarting `termux-usbmuxd` and then `idevicepair pair` again. Make sure your iOS device is unlocked and trusts the computer.
-   **`./idevice-tools pair` (or any other Rust `idevice-tools` subcommand) crashes with `thread 'main' panicked ... Bad USBMUXD_SOCKET_ADDRESS: AddrParseError(Socket)`**: This means your **current shell** has `USBMUXD_SOCKET_ADDRESS` set to something the Rust `idevice` crate can't parse - most commonly a leftover `UNIX:$PREFIX/var/run/usbmuxd` value from an older install of this project or from a classic libimobiledevice tutorial. That syntax is correct for `ideviceinfo`/`idevicepair` (C, libimobiledevice) but the Rust crate does **not** understand the `UNIX:` prefix at all: if the value contains a `:` it must be a plain, strictly valid `host:port` TCP address, nothing else.
    - Fix it for new shells: run `termux-usbmuxd doctor` (or menu option `5`). It rewrites `~/.bashrc`/`~/.zshrc` to the correct `export USBMUXD_SOCKET_ADDRESS=127.0.0.1:27015`.
    - Fix it for the shell you're in *right now* (rc-file changes don't retroactively affect an already-running shell): run
      ```bash
      export USBMUXD_SOCKET_ADDRESS=127.0.0.1:27015
      ```
      or simply close the Termux tab/session and open a new one, then retry.
    - `127.0.0.1:27015` (the TCP port that `termux-usbmuxd` bridges to the real Unix socket via `socat`) is the only value understood identically by both the C tools and the Rust tools, so don't switch it back to a `UNIX:` path.

## Contributing

Feel free to open issues or pull requests on the GitHub repository. Your contributions are welcome!

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
