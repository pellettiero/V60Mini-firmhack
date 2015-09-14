#include "os.h"
#include "snake.h"

/*
Main file for the hack. All intercepted hooks call routines here.
*/
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */



//How long should the flash be enabled after fn+f is pressed?
//10000 is about 6.7 seconds
#define FLASH_TIMER 14920 //10 sec

int flashEnabledTimer=0;

//First thing called after firmware starts up
void setup() {
	snakeSetup();
}

//Called when a key is pressed
//Uses USB scan codes - http://www.mindrunway.ru/IgorPlHex/USBKeyScan.pdf
void hookkeydown(int key) {
	snakeKeydown(key);
	//Check if fn-f is pressed
	if (keyBitmap[4]==0xf7 && keyBitmap[0xb]==0xdf) flashEnabledTimer=FLASH_TIMER;
}

//Called x times every second, in the idle loop of the keyboard
void hooktick() {
	snakeTick();
	if (flashEnabledTimer!=0) {
		flashEnabledTimer--;
		ledIntensity[0]=flashEnabledTimer?0:0xfff;
	}
}

//Usb fn 5 is stubbed in the original firmware, so we can patch in a routine to control the LEDs over
//USB here. This is that routine.
int dousbfn5(int subfn, int startAddr, int endAddr) {
	int x;
	short *data=(short*)0x20000B50; //Received data from USB. ToDo: use address in ld file for this
	if (endAddr>0x34) endAddr=0x34;
	for (x=startAddr; x<startAddr+endAddr; x++) {
		if (x<144) ledIntensity[x]=*data;
		data++;
	}
	return 0x4f; //because the original stub did this too
}


//Usb fn 4, the fn that makes the keyboard go into flash mode, is hooked and redirects here.
//This routine only makes it go into flash mode when fn+F is pressed beforehand.
int doflash(int subfn, int startAddr, int endAddr) {
	if (flashEnabledTimer!=0) {
		//Go to flash mode
		return flashorg(subfn, startAddr, endAddr);
	}
	//Don't flash.
	return 0x4f;
}

