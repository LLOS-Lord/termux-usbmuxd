# Termux-Usbmuxd (Enhanced)

## Table of Contents

1. [Introduction](#introduction)

1. [Original Repository Analysis](#original-repository-analysis)

1. [Architecture and Working Mechanism](#architecture-and-working-mechanism)
  - [usbmuxd and libimobiledevice](#usbmuxd-and-libimobiledevice)
  - [Termux USB API and File Descriptor](#termux-usb-api-and-file-descriptor)
  - [Rootless Challenges](#rootless-challenges)

1. [Enhanced Design](#enhanced-design)
  - [Optimized Auto-Detection](#optimized-auto-detection)
  - [File Descriptor Passing Mechanism](#file-descriptor-passing-mechanism)
  - [Stability and Compatibility Improvements](#stability-and-compatibility-improvements)

1. [Installation](#installation)

1. [Usage](#usage)

1. [Other Useful Commands](#other-useful-commands)

1. [Troubleshooting](#troubleshooting)

1. [References](#references)

## Introduction

This `termux-usbmuxd` repository is an enhanced version designed to run `usbmuxd` without root privileges on Android's Termux environment by leveraging `termux-usb`. The primary goal is to provide a stable and highly compatible solution for communicating with iOS devices (iPhone, iPad) from Termux, extending the capabilities of managing and interacting with Apple devices without requiring the Android device to be rooted.

## Original Repository Analysis

The original repository `vuphan0246810/termux-usbmuxd` took a significant step in enabling `usbmuxd` on Termux. Its strengths include:

- **Auto-Detection**: The script attempts to automatically find Apple devices by analyzing the output of `termux-usb -l` and `termux-usb -i` for Apple's `idVendor` (`05ac`).

- **Environment Integration**: Automatically adds `USBMUXD_SOCKET_ADDRESS` to `.bashrc` so that `libimobiledevice` tools can locate the `usbmuxd` socket.

- **Use of ****`termux-usb -r -E -e`**: This is the crucial mechanism for `termux-usb` to grant USB access to a child process (`usbmuxd`) without root by passing a USB device file descriptor (FD) into the child process's environment.

However, there were areas for optimization:

- **Auto-Detection Reliability**: Parsing JSON with `python3 -c` may not always be robust. A more specialized JSON parsing tool or better error handling is needed.

- **FD Passing Mechanism**: While `termux-usb -E -e` passes the FD via the `$FD` environment variable, standard `usbmuxd` isn't designed to receive an FD this way. It usually initializes `libusb` and scans for devices itself. This can lead to permission issues if the internal `libusb` cannot use the passed FD.

- **Startup Optimization**: Ensuring `usbmuxd` starts quickly and stably, especially when handling cases where devices are not found or access is denied.

## Architecture and Working Mechanism

### usbmuxd and libimobiledevice

- **usbmuxd**: A daemon (background process) responsible for multiplexing connections over USB to iOS devices. It listens for USB attach/detach events and provides a UNIX (or TCP) socket for other applications to communicate with iOS devices [4].

- **libimobiledevice**: A software library that allows applications to communicate with native iOS devices. It uses the `usbmuxd` socket to establish connections and perform tasks like app installation, device info access, etc. [1].

### Termux USB API and File Descriptor

Termux provides `termux-usb` to access USB devices from the Termux environment without root. The mechanism is as follows:

1. **List Devices**: `termux-usb -l` lists connected USB devices in JSON format.

1. **Open Device**: When a Termux app needs access to a specific USB device, it calls `termux-usb -r -E -e <command> <device_path>`. This requests USB access from Android. If granted, `termux-usb` opens the device and passes a file descriptor (FD) into the `$FD` environment variable of the `<command>` process [3] [7].

1. **Use FD**: The `<command>` process (in this case, `usbmuxd`) can then use this FD to communicate with the USB device via the `libusb` library.

### Rootless Challenges

The biggest challenge is that `usbmuxd` (and the `libusb` it uses) typically expects direct access to USB devices via `/dev/bus/usb`, which requires root. However, `termux-usb` provides a pre-authorized FD, allowing `libusb` to bypass opening the device itself [3] [7]. Specifically, `libusb` has a function `libusb_wrap_sys_device` [6] to initialize a handle from a system file descriptor.

## Enhanced Design

### Optimized Auto-Detection

The `termux-usbmuxd` script has been improved to handle `termux-usb -l` output more robustly and provides manual device selection if auto-detection fails.

### File Descriptor Passing Mechanism

We ensure `usbmuxd` uses the FD passed by `termux-usb` by using a specialized `usbmuxd_proxy` wrapper. This wrapper receives the FD and sets the `LIBUSB_FD` environment variable, which `libusb` on Android uses to recognize the pre-opened device [3] [7].

### Stability and Compatibility Improvements

- **Error Handling**: Added robust checks to inform users of issues like denied permissions or missing devices.

- **Process Management**: Uses a "Fork-and-Wait" mechanism in the proxy to keep the USB connection alive and prevents the script from accidentally killing itself using refined `pkill` logic.

- **Compatibility**: Verified that tools like `idevicepair`, `ideviceinfo`, and `ideviceinstaller` work seamlessly.

## Installation

To install this enhanced version, follow these steps:

1. **Install Prerequisites**:
  - Install **Termux** and **Termux:API** from F-Droid.
  - Download and install **Termux:API APK** (Version 51) to ensure compatibility: [Download Link](https://d.apkpure.com/b/APK/com.termux.api?versionCode=51)
    - In Termux, update packages and install required tools:
    
       ```bash
       pkg update && pkg upgrade -y
       pkg install -y git clang build-essential libusb libimobiledevice termux-api jq
       ```

1. **Clone Repository**:

   ```bash
   git clone https://github.com/vuphan0246810/termux-usbmuxd
   cd termux-usbmuxd
   ```

1. **Build and Install**:
 ```bash
   bash install.sh
   ```

## Usage

1. **Connect iPhone** via USB-OTG cable.

1. **Run the manager**:

   ```bash
   termux-usbmuxd
   ```

   *Note*: On first run, an Android dialog will ask for USB permission. Select "OK" (and "Always allow" if desired ).

1. **Pairing**: If it's your first time connecting:

   ```bash
   idevicepair pair
   ```

   Tap "Trust" on your iPhone screen.

1. **Check Connection**:

   ```bash
   idevicename
   ```

## Other Useful Commands

- `ideviceinfo`: Detailed iOS device information.

- `ideviceinstaller -l`: List installed apps.

- `ideviceinstaller -i app.ipa`: Install an IPA file.

- `idevicebackup2 backup --full <dir>`: Full device backup.

## Troubleshooting

- **No Device Found**:
    1. Check USB-OTG and Lightning cables.
    1. Re-plug the cable.
    1. Run `termux-usb -l` to see if Android detects the iPhone.

- **usbmuxd fails to start**:
    1. Check logs: `cat ~/.termux-usbmuxd.log`.
    1. Ensure `termux-api` is correctly installed and permissions are granted.

- **idevicepair fails**:
    1. Ensure iPhone is unlocked and you tapped "Trust".
    1. Restart `termux-usbmuxd`.

## References

[1]: https://github.com/libimobiledevice/libimobiledevice/issues/1487 "https://github.com/libimobiledevice/libimobiledevice/issues/1487"

[2]: https://forum.f-droid.org/t/usbmuxd-for-accessing-ios-devices-from-android-devices/31540 "https://forum.f-droid.org/t/usbmuxd-for-accessing-ios-devices-from-android-devices/31540"

[3]: https://www.reddit.com/r/termux/comments/1rimv1j/do_i_really_need_root_to_access_usb_hardware_in/ "https://www.reddit.com/r/termux/comments/1rimv1j/do_i_really_need_root_to_access_usb_hardware_in/"

[4]: https://github.com/libimobiledevice/usbmuxd "https://github.com/libimobiledevice/usbmuxd"

[5]: https://man.archlinux.org/man/usbmuxd.8.en "https://man.archlinux.org/man/usbmuxd.8.en"

[6]: https://libusb.sourceforge.io/api-1.0/group__libusb__dev.html#ga7030588820f4f039144458f27891369c "https://libusb.sourceforge.io/api-1.0/group__libusb__dev.html#ga7030588820f4f039144458f27891369c"

[7]: https://github.com/Querela/termux-usb-python "https://github.com/Querela/termux-usb-python"
