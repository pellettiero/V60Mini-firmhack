# KBParadise V60Mini Custom Firmware Project
A project to disassemble the KBParadise V60Mini keyboard firmware.

With help from the following people:

* Jeroen ([Spritetm](https://www.reddit.com/user/Spritetm)) at http://spritesmods.com/
* Charlie https://github.com/ChaoticConundrum
* and others...

Without your help, this project would not have been possible. Thanks everyone!

# WARNING
**I take ABSOLUTELY NO responsibilities if your keyboard turns into an expensive brick! Use this at your own risk.**

# Usage

**First, make sure your keyboard is:**
* **ANSI US version**
* **on firmware 1.0.7** *(WIP: might not be required anymore)*  

**THIS IS VERY IMPORTANT!**  
**Use the updater from Windows to update the keyboard if not already done.**  
*(WIP Note: this might not be necessary anymore)*

You can find the original and patched version of the firmware exe in the `firmware/` folder, along with their SHA1 sums:  
https://github.com/pellettiero/V60Mini-firmhack/tree/master/firmware    

Run this on Windows as Administrator to unlock the keyboard.  
***Remember to check the SHA1 sums just to be sure to avoid a brick.***

# Disassemble and Unlock

*If you want to do it the manual way, or just to disassemble the firmware itself:*


* If you want to **decrypt the updater**:
    ```
    $ updater_decrypt.sh cykb112_v107.exe
    ```
    You can disassemble the decrypted file with `disassemble.sh`.

* If you instead want to **unlock the keyboard**:
    ```
    $ updater_patcher.sh cykb112_v107.exe
    ```
    Get the output executable from the directory and run it on Windows with admin
    permissions.  
    Fingers crossed!
    *(Still working on a way to do this from Linux itself)*

    The patched updater is checked with SHA1 to avoid arbitrary fuckups.  
    Check this issue to get an explanation of how the unlock works:      
    https://github.com/pok3r-custom/pok3r_re_firmware/issues/4

# Flash backup

You can then try to dump the flash by using `pok3rtool`:  
Prerequisites: `cmake git libusb libusb-compat`

1. Clone the repo
```
$ git clone https://github.com/pok3r-custom/pok3rtool
```
2. Build
```
$ cd pok3rtool
$ git submodule update --init && cmake . && make pok3rtool
```
It will take a while, be patient.

3. Reboot into bootloader
```
$ sudo ./pok3rtool -t kbpv60 bootloader
```

4. Dump flash and reboot to firmware
```
$ sudo ./pok3rtool -t kbpv60 dump flash.bin
$ sudo ./pok3rtool -t kbpv60 reboot
```
Now you have a backup of the entire flash! Store this in a safe place, just in case of bricks.  
It can be restored using a JTAG programmer/debugger.

# Set a firmware version
You also might want to set a different firmware version to remind yourself this keyboard is now unlocked.  
To do so:
```
$ sudo ./pok3rtool -t kbpv60 bootloader
$ sudo ./pok3rtool -t kbpv60 setversion 1.0.7u
$ sudo ./pok3rtool -t kbpv60 reboot
```
Just remember the small `u` means *"unlocked"*.  
You'll be able to check the version using  
`sudo ./pok3rtool list`  
or   
`sudo ./pok3rtool -t kbpv60 version`  
*(this command might only work with the keyboard in bootloader mode)*

## Roadmap
 - [x] Find a way to decrypt firmware from updater
 - [x] Mod updater to unlock keyboard and allow reading flash
 - [x] Unlock keyboard and dump flash
 - [x] Find bootloader XOR encryption key in disassembled bootloader *(same as pok3r)*
 - [x] ~~Find out how the USB packets are encrypted~~ No need, works natively with pok3r commands
 - [ ] Custom firmware? **WIP**

## Notes
* Official flashing tool compares last two bytes of firmware to find the XOR encryption key
* 0x0023c1 contains the following string, where each letter takes up a byte: "USB-HID Keyboard"

## Bugs
* When DIP switch 6 is ON (switches FN with ALTGR and MENU with FN), pressing FN+ENTER (arrow mode) overwrites the FN function with the DOWN arrow. No way to exit this mode unless DIP switch 6 is returned to original position.
