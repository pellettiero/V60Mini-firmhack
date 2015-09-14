Rapiditool + rapidisnake

This is a set of tools to modify a Coolermaster Quickshot Rapid-I keyboard, plus an example that
adds a few features to the existing firmware: a way to upload the PWM values of the keyboard
using USB, a safety so not everyone can flash your keyboard without having physical access to it,
and finally, a built-in version of Snake.

BIG FAT WARNING:
This code comes under no warranty whatsoever. It's pretty possible to kill your keyboard using this, and
I'm pretty sure it also voids your warranty, so there is a slight possibility you can turn your 
keyboard into something worth not much more than a boat anchor. In that case, I nor Coolermaster
nor anyone else will be responsible for any losses or damages you suffer. In other words: if you
break your keyboard, you get to keep both pieces.

Now that's out of the way, how do you use this? You will need:
- Linux
- Dependencies. Off the top of my head, the special things it needs are
  arm-none-eabi-gcc, gcc, libusb1.0-dev, make and bash
- A Rapid-I keyboard. Duh.

First of all, you'll need the original firmware from Coolermaster. Go download the 1.1.7
firmware update from http://gaming.coolermaster.com/en/products/keyboards/rapid-i/
It's under driver -> downloads -> Firmware V.117 (US ANSI layouts, 87 keys)
For now, the hack only supports the US version. Modifying it for the EU version should be
pretty trivial if you know what you're doing, but I don't have a keyboard like that here.
I also don't know if flashing the US firmware on an EU keyboard will break it; I *think*
it will most likely only slightly mess up keyboard mappings, but I haven't been able to
try it yet. Unpack the zip and dump the exe in this directory.

Next, you'll need to extract the firmware code from the exe. This is pretty easy: just run
getflashimg.sh in this directory to do that.

Finally, go to the 'hack' directory and run 'make'. This builds the modified firmware image with all 
the nice extra features in it. Make sure the 'make' session doesn't end in an error.

Up till now, your keyboard hasn't been touched and is still running the stock firmware. That's 
gonna change with the next step. This is a dangerous step; don't proceed any further if you're
not okay with the (small, but existent) risk that this can break your keyboard.

Okay, you're prepared and ready? Now, as root, run 'make flash'. (eg by doing 'sudo make flash').
The program should be busy for a few seconds, and your keyboard should be running the modified firmware
now! One way to check this is to press fn+f, the 'esc' key should now light up for 10 seconds. 
(By the way, you need to do this when you are updating your firmware: with the patched firmware,
flashing only works if you start it within 10 seconds after you've pressed fn+f)

If all you want to do is play Snake, you're done now. If you want to develop, you'd better make a
backup of the entire flash of the keyboard now. You do this by going to the rapiditool directory
and running './rapiditool -readflash flash_backup.bin'. Now, if you manage to screw up the firmware,
you can always open up your keyboard, bridge the USB bootloader header and flash back this image 
with the Holtek tools.

Have fun!
