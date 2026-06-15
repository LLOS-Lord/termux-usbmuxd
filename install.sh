#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

REPO="usedoperative-sudo/termux-usbmuxd"
BRANCH="main"

echo "==> Installing termux-usbmuxd..."

echo "    installing dependencies..."
pkg install -y usbmuxd libimobiledevice termux-api 2>/dev/null

echo "    downloading termux-usbmuxd..."
curl -sL "https://raw.githubusercontent.com/$REPO/$BRANCH/termux-usbmuxd" \
  -o "$PREFIX/bin/termux-usbmuxd"

chmod +x "$PREFIX/bin/termux-usbmuxd"

echo "==> installed to $PREFIX/bin/termux-usbmuxd"
echo ""
echo "Quick start:"
echo "  1. Connect iPhone via USB-OTG"
echo "  2. termux-usb -l          # find device path"
echo "  3. termux-usbmuxd /dev/bus/usb/001/002"
echo "  4. export USBMUXD_SOCKET_ADDRESS=UNIX:\$PREFIX/var/run/usbmuxd"
echo "  5. idevicepair pair       # trust on iPhone"
echo "  6. ideviceinstaller -i app.ipa"
