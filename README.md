# termux-usbmuxd

Run `usbmuxd` on unrooted Termux to communicate with iOS devices over USB.

No root required. Works by passing USB device access through `termux-usb` (Termux:API).

## Requirements

- Android device with USB-OTG support
- [Termux](https://f-droid.org/packages/com.termux/) (from F-Droid)
- [Termux:API](https://f-droid.org/packages/com.termux.api/) (from F-Droid)
- `usbmuxd` and `libimobiledevice` packages installed in Termux
- USB OTG cable/adapter to connect your iPhone

## Installation

```bash
pkg install usbmuxd libimobiledevice termux-api
git clone https://github.com/usedoperative-sudo/termux-usbmuxd
cd termux-usbmuxd
chmod +x termux-usbmuxd
cp termux-usbmuxd $PREFIX/bin/
```

Or install directly:

```bash
curl -sL https://raw.githubusercontent.com/usedoperative-sudo/termux-usbmuxd/main/install.sh | bash
```

## Usage

### 1. Connect your iPhone via USB-OTG

### 2. Find your device

```bash
termux-usb -l
```

Example output: `/dev/bus/usb/001/002`

### 3. Start usbmuxd

```bash
termux-usbmuxd /dev/bus/usb/001/002
```

You should see:

```
==> starting usbmuxd for /dev/bus/usb/001/002
    socket: /data/data/com.termux/files/usr/var/run/usbmuxd
    pidfile: /data/data/com.termux/files/usr/var/run/usbmuxd.pid
    pairing: /data/data/com.termux/files/usr/var/lib/lockdown
==> usbmuxd running (pid 12345)
    socket: /data/data/com.termux/files/usr/var/run/usbmuxd
    export USBMUXD_SOCKET_ADDRESS=UNIX:/data/data/com.termux/files/usr/var/run/usbmuxd
```

### 4. Set the socket address

```bash
export USBMUXD_SOCKET_ADDRESS=UNIX:$PREFIX/var/run/usbmuxd
```

### 5. Pair with your iPhone

```bash
idevicepair pair
```

Unlock your iPhone and tap "Trust This Computer" when prompted then execute the command again.

### 6. Verify connection

```bash
ideviceinfo
```

With this tool you can install SideStore to sideload wirelessly from your Android device. (More info [here](https://github.com/usedoperative-sudo/termux-usbmuxd/blob/main/Step-To-Step.md))

## Options

| Option | Description | Default |
|--------|-------------|---------|
| `-s, --socket ADDR` | Socket address (TCP: `127.0.0.1:27015`) | `$PREFIX/var/run/usbmuxd` |
| `-p, --pidfile PATH` | PID file path (`NONE` to disable) | `$PREFIX/var/run/usbmuxd.pid` |
| `-f, --foreground` | Run in foreground | background |
| `-v, --verbose` | Verbose output | off |

## How it works

Normally `usbmuxd` needs root to access `/dev/bus/usb/*`. On Android, Termux is sandboxed and can't access these device files.

Termux:API provides `termux-usb`, which uses Android's USB API to claim a USB device. With the `-E` flag, it can execute a child process with the USB file descriptor accessible, effectively giving `usbmuxd` the USB access it needs without root.

Since Termux's `usbmuxd` package is already compiled with `$PREFIX` paths, the socket and pairing directories work out of the box. This wrapper focuses on providing the USB device access through `termux-usb`.

## License

MIT
