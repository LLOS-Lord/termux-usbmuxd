# Setup

Here we will see how to install and use the wrapper `termux-usbmuxd` in a Termux environment to achieve interaction with a iOS device without root access, made by Used (me), the indispensable preconditions are: by logic, a Android device and a iDevice, Termux and Termux:API apps in their F-Droid versions, PRoot-Distro (optional, just for sideload) and graphics server viewer such as RealVNC or Termux:X11 (optional too, you can use CLI software to interact with your iDevice)

- Run this in Termux: `curl -sL https://raw.githubusercontent.com/usedoperative-sudo/termux-usbmuxd/main/install.sh | bash` this will install everything for running `usbmuxd` automatically
Make sure you have the Termux:API app installed before continuing
Test the Termux:API with the command `termux-api-start`
If it returns `Starting service: Intent { cmp=com.termux.api/.KeepAliveService }` means that the API works correctly
Connect your iDevice to Android and run `termux-usb -l` to verify USB detection, if it returns this:
```log
[
  "/dev/bus/usb/001/002"
]
```
Where 001 and 002 can change for any combination of 3 numbers, the iDevice was detected correctly, in my case I will use `/dev/bus/usb/001/002`

# Starting `usbmuxd` in Termux and pairing a iDevice

Now that `usbmuxd` and the wrapper `termux-usbmuxd` have already been installed in the system, we will initialize the connection between the Android and the iDevice

- With the iDevice connected to Android run `termux-usbmuxd /dev/bus/usb/001/002`, seconds later Android will ask for permission to allow access to the USB device to the Termux:API, this must be granted or else the pairing will fail
- Once it is confirmed that `usbmuxd` is running, with the iDevice unlocked you can run `idevicepair pair` so that the trust alert appears in iOS, once the unlock key is entered and the alert disappears, you must run `idevicepair pair` again to end the pairing with Termux, once paired, you can try commands such as `idevicename`, `ideviceinfo`, `idevicedate`, `idevice_id`, etc. to verify the functionality of the pairing file

# Sideloading on the iDevice from a PRoot-Distro

This part is already optional, but if what you want is to achieve lateral loading on your iDevice, this is very important and you must follow the steps very carefully, for this example I will use iLoader as a sideload reference but you can use any software to interact with your iDevice as long as it has a version for Linux over ARM64

- Install PRoot-Distro if you don't already have it with the command `pkg install proot-distro`
- Once installed, install any distro between Debian and Ubuntu with `pd i distro` where `distro` is the one you choose, For this example I will use Ubuntu
-  Enter the distro you chose with the following command `pd sh ubuntu --bind $PREFIX/var/run:/var/run --bind $PREFIX/var/lib/lockdown:/var/lib/lockdown --shared-tmp`

> [!WARNING]
> No command arguments should be ignored when entering the distro, otherwise there may be errors, such as that the graphic apps do not work inside the PRoot or that the socket of `usbmuxd` or the pairing file are not found

- Once inside the distro you should have your terminal prompt changed from `~ $` to `root@localhost:~#` that means that you have entered the distro emulated with the user `root`
- Download iLoader with the following command `wget https://github.com/nab138/iloader/releases/latest/download/iloader-linux-arm64.deb` and install it with `dpkg -i ./iloader-linux-arm64.deb`
- Now start the graphic server of your choice, either a VNC server with the command `vncserver :0` inside the PRoot or Termux:X11 with the command `termux-x11 -ac :0` from another Termux tab directly, once that is done, export DISPLAY with `export DISPLAY=:0` and run your preferred desktop environment
- Start the desktop environment in the terminal inside the PRoot and then go to your graphic server viewer, once in the interface, open the app drawer and look for iLoader to run it, for example, in XFCE4, iLoader is in the "Accessories" section of the application drawer
- Finally, once in the iLoader interface log in with your Apple ID, your iPhone will believe that a MacBook Pro in your location tries to access with your Apple ID at the time of two-step verification, this is because iLoader uses Xcode data to make Apple servers think you are a developer testing their own apps, this data is called Anisette, and select the SideStore variant that you prefer or install external `.ipa` files from there

# Finishing

This part serves as a closure and has a single step that serves to know what to do once you finish interacting with the iDevice by USB

- When you finish interacting with your iDevice with the software you have chosen, you just have to disconnect the iDevice from the Android device, run `pkill usbmuxd` to kill any orphaned process of the daemon and close Termux to end the session