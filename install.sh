#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

echo "==> Installing improved termux-usbmuxd..."

echo "    Installing dependencies..."
pkg update && pkg upgrade -y
pkg install -y usbmuxd libimobiledevice which termux-api build-essential clang python jq libzip

git clone https://github.com/LLOS-Lord/ideviceinstaller.git
cd ideviceinstaller

./autogen.sh --prefix=$PREFIX

make clean

make
make install

cd ..

echo "    Compiling usbmuxd_proxy..."
gcc usbmuxd_proxy.c -o "$PREFIX/bin/usbmuxd_proxy" || {
    echo "Error: Failed to compile usbmuxd_proxy." >&2
    exit 1
}
chmod +x "$PREFIX/bin/usbmuxd_proxy"

echo "    Setting up termux-usbmuxd script..."
cp termux-usbmuxd "$PREFIX/bin/termux-usbmuxd"
chmod +x "$PREFIX/bin/termux-usbmuxd"

BASHRC="$HOME/.bashrc"
SOCKET_LINE="export USBMUXD_SOCKET_ADDRESS=UNIX:\$PREFIX/var/run/usbmuxd"

if ! grep -q "USBMUXD_SOCKET_ADDRESS" "$BASHRC"; then
    echo "    Adding environment variable to .bashrc..."
    echo "" >> "$BASHRC"
    echo "# termux-usbmuxd configuration" >> "$BASHRC"
    echo "$SOCKET_LINE" >> "$BASHRC"
fi

echo "==> Installation complete!"
echo ""
echo "Quick start:"
echo "  1. Connect iPhone via USB-OTG"
echo "  2. Run: termux-usbmuxd"
echo "  3. (First time) Grant USB permission in the Android popup"
echo "  4. Run: idevicepair pair"

cd
