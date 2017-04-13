# KBParadise V60Mini Custom Firmware Project
A project to disassemble the KBParadise V60Mini keyboard firmware.

With help from the following people:

* Jeroen ([Spritetm](https://www.reddit.com/user/Spritetm)) at http://spritesmods.com/
* Charlie https://github.com/ChaoticConundrum
* and others...
Without your help, this project would not have been possible. Thanks everyone!

### WARNING! USE THIS AT YOUR OWN RISK!

## Roadmap
[x] Find a way to decrypt firmware from updater
[x] Mod updater to unlock keyboard and permit flash being read
[x] Unlock keyboard and dump flash
[] Find bootloader XOR encryption key in disassembled bootloader

## Notes
* Official flashing tool compares last two bytes of firmware to find the XOR encryption key
* 0x0023c1 contains the following string, where each letter takes up a byte: "USB-HID Keyboard"


## Bugs
* When DIP switch 6 is ON (switches FN with ALTGR and MENU with FN), pressing FN+ENTER (arrow mode) overwrites the FN function with the DOWN arrow. No way to exit this mode unless DIP switch 6 is returned to original position.

