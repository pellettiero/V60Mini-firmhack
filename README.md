# V60Mini Firmware Hacking (WIP)
A project to disassemble the KBParadise V60 Mini keyboard firmware.

With help from Jeroen ([Spritetm](https://www.reddit.com/user/Spritetm)) at http://spritesmods.com/.   
Without his help, this project would not have been possible. Thanks man!

##NOTES##
* Official flashing tool compares last two bytes of firmware to find the XOR encryption key
* 0x0023c1 contains the following string, where each letter takes up a byte: "USB-HID Keyboard"

##TODOS AND STUFF
**BUG** When DIP switch 6 is ON (switch FN with ALTGR and MENU with FN), pressing FN+ENTER (arrow mode) overwrites the FN function with the DOWN arrow. No way to exit this mode unless DIP switch 6 is returned to original position.

