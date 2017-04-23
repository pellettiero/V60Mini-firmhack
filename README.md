# KBParadise V60Mini Custom Firmware Project
A project to disassemble the KBParadise V60Mini keyboard firmware.

With help from the following people:

* Jeroen ([Spritetm](https://www.reddit.com/user/Spritetm)) at http://spritesmods.com/
* Charlie https://github.com/ChaoticConundrum
* and others...

Without your help, this project would not have been possible. Thanks everyone!

# !WARNING! USE THIS AT YOUR OWN RISK!
I take ABSOLUTELY NO responsibilities if your keyboard turns into an expensive
brick!
# !WARNING! USE THIS AT YOUR OWN RISK!

## Usage & How To
**First, make sure your keyboard is on firmware 1.0.7! THIS IS VERY IMPORTANT!**
Use the updater from Windows to update the keyboard if not already done.

You need the original v60Mini updater executable **(v1.0.7)** from the Facebook page to do the
following.

*TODO: Download this automatically?*

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
    https://github.com/ChaoticConundrum/pok3r_re_firmware/issues/4

    You can then try to dump the flash by using my modified fork of ChaoticConundrum's `pok3rtool`:       
    1. Clone the repo
    ```
    $ git clone https://github.com/pellettiero/pok3rtool-v60mini
    ```
    2. Build
    ```
    $ cd pok3rtool-v60mini
    $ git submodule update --init && cmake . && make
    ```
    3. Dump flash
    ```
    $ sudo ./pok3rtool -t pok3r dump flash.bin
    ```

## Roadmap
 - [x] Find a way to decrypt firmware from updater
 - [x] Mod updater to unlock keyboard and permit flash being read
 - [x] Unlock keyboard and dump flash
 - [x] Find bootloader XOR encryption key in disassembled bootloader
 - [ ] Find out how the USB packets are encrypted
 - [ ] Custom firmware?

## Notes
* Official flashing tool compares last two bytes of firmware to find the XOR encryption key
* 0x0023c1 contains the following string, where each letter takes up a byte: "USB-HID Keyboard"

## Bugs
* When DIP switch 6 is ON (switches FN with ALTGR and MENU with FN), pressing FN+ENTER (arrow mode) overwrites the FN function with the DOWN arrow. No way to exit this mode unless DIP switch 6 is returned to original position.
