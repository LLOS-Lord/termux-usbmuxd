#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

echo "==> Installing improved termux-usbmuxd..."

echo "    Installing dependencies..."
pkg update && pkg upgrade -y
pkg install -y usbmuxd libimobiledevice which termux-api build-essential clang python jq libzip socat

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

# Use the TCP form (not "UNIX:/path"). Rust-based tools like idevice-tools parse this
# variable themselves: any value containing a colon is treated as a TCP host:port and
# fed straight to a strict SocketAddr parser, so "UNIX:/path" panics with
# "Bad USBMUXD_SOCKET_ADDRESS: AddrParseError(Socket)". 127.0.0.1:27015 works for both
# classic libimobiledevice tools and idevice-tools since termux-usbmuxd bridges it to
# the real Unix socket via socat.
SOCKET_LINE="export USBMUXD_SOCKET_ADDRESS=127.0.0.1:27015"

# IMPORTANT: this must be idempotent AND self-healing, not just "add if
# missing". If the user previously installed this project (or followed any
# other Termux/libimobiledevice guide) with the classic
# "UNIX:$PREFIX/var/run/usbmuxd" form - which is the CORRECT syntax for
# libimobiledevice's C tools but is NOT understood by Rust-based
# idevice-tools - a plain "grep -q ... || append" check would see the
# variable already mentioned and skip fixing it, silently leaving the
# broken value in place forever. So instead: always strip any existing
# USBMUXD_SOCKET_ADDRESS line(s) first, then (re)write the correct one.
configure_shell_rc() {
    local rc_file="$1"
    [ -f "$rc_file" ] || touch "$rc_file"
    if grep -q "USBMUXD_SOCKET_ADDRESS" "$rc_file"; then
        echo "    Found an existing USBMUXD_SOCKET_ADDRESS in $rc_file, normalizing it..."
    else
        echo "    Adding environment variables to $rc_file..."
    fi
    sed -i '/USBMUXD_SOCKET_ADDRESS/d' "$rc_file"
    sed -i '/# termux-usbmuxd configuration/d' "$rc_file"
    {
        echo ""
        echo "# termux-usbmuxd configuration"
        echo "$SOCKET_LINE"
    } >> "$rc_file"
}

configure_shell_rc "$HOME/.bashrc"
[ -f "$HOME/.zshrc" ] && configure_shell_rc "$HOME/.zshrc"

echo "==> Installation complete!"
echo ""
echo "Quick start:"
echo "  1. Connect iPhone via USB-OTG"
echo "  2. Run: termux-usbmuxd"
echo "  3. (First time) Grant USB permission in the Android popup"
echo "  4. Run: idevicepair pair"
echo ""
echo "IMPORTANT - apply the new USBMUXD_SOCKET_ADDRESS to THIS shell right now"
echo "(the line was written to your rc file(s), but that only affects NEW"
echo "shells - this one is already running):"
echo ""
echo "  source ~/.bashrc"
echo ""
echo "or just close this Termux session/tab and open a new one. Otherwise"
echo "standalone Rust tools like idevice-tools may still crash with:"
echo "  Bad USBMUXD_SOCKET_ADDRESS: AddrParseError(Socket)"
echo "until you do this."

cd
