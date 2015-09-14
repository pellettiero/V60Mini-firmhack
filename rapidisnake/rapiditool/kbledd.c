/*
This is an example bit of code that uses the fn5 usb function in the hacked firmware to pass
LED bitmaps to the keyboard. This specific example runs a fluid simulation. It uses the Linux
event interface (/dev/input/eventX) to get keypresses, converts that to a location and uses
a fluid simulation to make ripples starting from that point. It'll then send the fluid bitmap
over to the keyboard. The net effect is a 'puddle simulator' where pressing a key has the same 
effect of throwing a pebble in a puddle.

This code is pretty Linux-specific, but the general idea is portable to most OSses.
*/

/*
(C) 2014 Jeroen Domburg (jeroen AT spritesmods.com)

This program is free software: you can redistribute it and/or modify
t under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <stdlib.h>
#include <libusb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

//Definition of a keycap
typedef struct {
	int id;		//position of key in PWM bitmap
	float x;	//Location X, in keycap-widths
	float y;	//Location Y, in keycap-heights
	int keycode; //Linux input subsystem keycode
} KeyDef;

//Keycap definition for the Rapid-I keyboard
KeyDef keys[]={
	{0, 0, 0, KEY_ESC},
	{8, 2, 0, KEY_F1},
	{16, 3, 0, KEY_F2},
	{24, 4, 0, KEY_F3},
	{32, 5, 0, KEY_F4},
	{40, 6.5, 0, KEY_F5},
	{48, 7.5, 0, KEY_F6},
	{56, 8.5, 0, KEY_F7},
	{64, 9.5, 0, KEY_F8},
	{72, 11, 0, KEY_F9},
	{80, 12, 0, KEY_F10},
	{88, 13, 0, KEY_F11},
	{96, 14, 0, KEY_F12},
	{14, 17.2, 0, KEY_PAUSE},

	{1, 0, 1, KEY_GRAVE},
	{9, 1, 1, KEY_1},
	{17, 2, 1, KEY_2},
	{25, 3, 1, KEY_3},
	{33, 4, 1, KEY_4},
	{41, 5, 1, KEY_5},
	{49, 6, 1, KEY_6},
	{57, 7, 1, KEY_7},
	{65, 8, 1, KEY_8},
	{73, 9, 1, KEY_9},
	{81, 10, 1, KEY_0},
	{89, 11, 1, KEY_MINUS},
	{97, 12, 1, KEY_EQUAL},
	{7, 14, 1, KEY_BACKSPACE},

	{2, 0.5, 2, KEY_TAB},
	{10, 1.5, 2, KEY_Q},
	{18, 2.5, 2, KEY_W},
	{26, 3.5, 2, KEY_E},
	{34, 4.5, 2, KEY_R},
	{42, 5.5, 2, KEY_T},
	{50, 6.5, 2, KEY_Y},
	{58, 7.5, 2, KEY_U},
	{66, 8.5, 2, KEY_I},
	{74, 9.5, 2, KEY_O},
	{82, 10.5, 2, KEY_P},
	{90, 11.5, 2, KEY_LEFTBRACE},
	{98, 12.5, 2, KEY_RIGHTBRACE},
	{106, 14, 2, KEY_BACKSLASH},

	{11, 1.75, 3, KEY_A},
	{19, 2.75, 3, KEY_S},
	{27, 3.75, 3, KEY_D},
	{35, 4.75, 3, KEY_F},
	{43, 5.75, 3, KEY_G},
	{51, 6.75, 3, KEY_H},
	{59, 7.75, 3, KEY_J},
	{67, 8.75, 3, KEY_K},
	{75, 9.75, 3, KEY_L},
	{83, 10.75, 3, KEY_SEMICOLON},
	{91, 11.75, 3, KEY_APOSTROPHE},
	{107, 13.5, 3, KEY_ENTER},

	{4, 0.75, 4, KEY_LEFTSHIFT},
	{20, 2.2, 4, KEY_Z},
	{28, 3.2, 4, KEY_X},
	{36, 4.2, 4, KEY_C},
	{44, 5.2, 4, KEY_V},
	{52, 6.2, 4, KEY_B},
	{60, 7.2, 4, KEY_N},
	{68, 8.2, 4, KEY_M},
	{76, 9.2, 4, KEY_COMMA},
	{84, 10.2, 4, KEY_DOT},
	{92, 11.2, 4, KEY_SLASH},
	{108, 13, 4, KEY_RIGHTSHIFT},

	{5, 0.1, 5, KEY_LEFTCTRL},
	{13, 1.5, 5, KEY_LEFTMETA},
	{21, 2.75, 5, KEY_LEFTALT},
	{45, 6.5, 5, KEY_SPACE},
	{77, 10, 5, KEY_RIGHTALT},
	{85, 11.5, 5, KEY_RIGHTMETA},
	{93, 12.7, 5, KEY_VOLUMEUP}, //function key
	{101, 13.9, 5, KEY_RIGHTCTRL},

	{23, 15.2, 1, KEY_INSERT},
	{22, 16.2, 1, KEY_HOME},
	{30, 17.2, 1, KEY_PAGEUP},

	{31, 15.2, 2, KEY_DELETE},
	{70, 16.2, 2, KEY_END},
	{78, 17.2, 2, KEY_PAGEDOWN},

	{102, 16.2, 4, KEY_UP},
	{109, 15.2, 5, KEY_LEFT},
	{110, 16.2, 5, KEY_DOWN},
	{111, 17.2, 5, KEY_RIGHT},

	{-1, 0, 0, 0}
};


libusb_context *ctx;
libusb_device_handle *dev;
int devInFlashMode=0;

void setint(char *b, int i){
	b[0]=(i>>0)&0xff;
	b[1]=(i>>8)&0xff;
	b[2]=(i>>16)&0xff;
	b[3]=(i>>24)&0xff;
}

void setword(char *b, int i){
	b[0]=(i>>0)&0xff;
	b[1]=(i>>8)&0xff;
}

//From http://mdfs.net/Info/Comp/Comms/CRC16.htm
#define poly 0x1021
int crc16(char *addr, int num) {
	int i;
	int crc=0;
	for (; num>0; num--)               /* Step through bytes in memory */
	{
		crc = crc ^ (*addr++ << 8);      /* Fetch byte from memory, XOR into CRC top byte*/
		for (i=0; i<8; i++)              /* Prepare to rotate 8 bits */
		{
			crc = crc << 1;                /* rotate */
			if (crc & 0x10000)             /* bit 15 was set (now bit 16)... */
			crc = (crc ^ poly) & 0xFFFF; /* XOR with XMODEM polynomic */
		                           /* and ensure CRC remains 16-bit value */
		}                              /* Loop for 8 bits */
	 }                                /* Loop until num=0 */
	return(crc);                     /* Return updated CRC */
}

//Fix the CRC of a packet to the keyboard
void fixcrc(char *buff, int len) {
	int c;
	setword(&buff[2], 0);
	c=crc16(buff, len);
	setword(&buff[2], c);
}

//Try and detach the interface we need on the keyboard from the HID driver
void tryDetachDev(libusb_device *dev, int iface) {
	int i, r, noport, bus;
	char portnums[8];
	char devid[128];
	char unbindfn[256];
	int config=1;
	FILE *f;
	bus=libusb_get_bus_number(dev);
	noport=libusb_get_port_numbers(dev, portnums, 8);
	//bus-port.port.port:config.interface
	sprintf(devid, "%d-", bus);
	for (i=0; i<noport; i++) {
		sprintf(devid+strlen(devid), "%s%hhu", (i==0)?"":".", portnums[i]);
	}
	sprintf(devid+strlen(devid),":%u.%u", config, iface);
	sprintf(unbindfn, "/sys/bus/usb/devices/%s/driver/unbind", devid);
//	printf("Unbind: echo %s > %s", devid, unbindfn);
	f=fopen(unbindfn, "w");
	if (f==NULL) {
		//Assume we already are unbound.
		return;
	}
	fprintf(f, "%s\n", devid);
	fclose(f);
	fprintf(stderr, "Unbound device iface from kernel.\n");
}

//Open the keyboard dev and detach from HID if needed
libusb_device_handle *usbOpenAndDetach(int vid, int pid, int iface) {
	int no, i, r;
	libusb_device_handle *ret=NULL;
	libusb_device **devs;
	libusb_device *dev;
	no=libusb_get_device_list(ctx, &devs);
	for (i=0; i<no; i++) {
		struct libusb_device_descriptor desc;
		dev=devs[i];
		r=libusb_get_device_descriptor(dev, &desc);
//		printf("%x %x\n", desc.idVendor, desc.idProduct);
		if (r==0 && desc.idVendor==vid && desc.idProduct==pid) {
			tryDetachDev(dev, iface);
			r=libusb_open(dev, &ret);
			if (r==0) {
				break;
			}
		}
	}
	libusb_free_device_list(devs, 1);
	return ret;
}



//Find and open the keyboard USB dev
void usbDevOpen() {
	libusb_init(&ctx);

	dev=usbOpenAndDetach(0x2516, 0x0020, 1);
	if (dev==NULL) {
		printf("opening device: none found");
		exit(0);
	}

	if (libusb_set_configuration(dev, 1)!=0) {
		//Don't complain; probably already set by usbhid
		//perror("setting config iface");
	}
	if (libusb_claim_interface(dev, 1)!=0) perror("claim iface");
}

void usbDevClose() {
	//libusb_attach_kernel_driver(dev, 1);
	libusb_close(dev);
	libusb_exit(ctx);
}


//Use usb fn5 packets to send the led pwm values to the keyboard
void sendLeds(int *leds, int off, int no) {
	int l, x, c;
	int num;
	char buff[64];

	while (no>0) {
		num=no;
		if (num>0x34/2) num=0x34/2;
//		printf("Off %d no %d num %d\n", off, no, num);

		memset(buff, 0, 64);
		buff[0]=5;
		buff[1]=0;
		setint(&buff[4], off);
		setint(&buff[8], num);
		for (x=0; x<num; x++) {
			setword(&buff[0xc+(x*2)], *leds++);
		}
		fixcrc(buff, 64);
		if (libusb_interrupt_transfer(dev, 0x04, buff, 64, &l, 1000)!=0) perror("int xfer w");
		no-=num;
		off+=num;
	}
	return;
}

//Intensity to PWM value conversion thing
int correctpwm(int x) {
	const pwmval[]={
	65535,    65508,    65479,    65451,    65422,    65394,    65365,    65337,
     65308,    65280,    65251,    65223,    65195,    65166,    65138,    65109,
     65081,    65052,    65024,    64995,    64967,    64938,    64909,    64878,
     64847,    64815,    64781,    64747,    64711,    64675,    64637,    64599,
     64559,    64518,    64476,    64433,    64389,    64344,    64297,    64249,
     64200,    64150,    64099,    64046,    63992,    63937,    63880,    63822,
     63763,    63702,    63640,    63577,    63512,    63446,    63379,    63310,
     63239,    63167,    63094,    63019,    62943,    62865,    62785,    62704,
     62621,    62537,    62451,    62364,    62275,    62184,    62092,    61998,
     61902,    61804,    61705,    61604,    61501,    61397,    61290,    61182,
     61072,    60961,    60847,    60732,    60614,    60495,    60374,    60251,
     60126,    59999,    59870,    59739,    59606,    59471,    59334,    59195,
     59053,    58910,    58765,    58618,    58468,    58316,    58163,    58007,
     57848,    57688,    57525,    57361,    57194,    57024,    56853,    56679,
     56503,    56324,    56143,    55960,    55774,    55586,    55396,    55203,
     55008,    54810,    54610,    54408,    54203,    53995,    53785,    53572,
     53357,    53140,    52919,    52696,    52471,    52243,    52012,    51778,
     51542,    51304,    51062,    50818,    50571,    50321,    50069,    49813,
    49555,    49295,    49031,    48764,    48495,    48223,    47948,    47670,
     47389,    47105,    46818,    46529,    46236,    45940,    45641,    45340,
     45035,    44727,    44416,    44102,    43785,    43465,    43142,    42815,
     42486,    42153,    41817,    41478,    41135,    40790,    40441,    40089,
     39733,    39375,    39013,    38647,    38279,    37907,    37531,    37153,
     36770,    36385,    35996,    35603,    35207,    34808,    34405,    33999,
     33589,    33175,    32758,    32338,    31913,    31486,    31054,    30619,
     30181,    29738,    29292,    28843,    28389,    27932,    27471,    27007,
     26539,    26066,    25590,    25111,    24627,    24140,    23649,    23153,
     22654,    22152,    21645,    21134,    20619,    20101,    19578,    19051,
     18521,    17986,    17447,    16905,    16358,    15807,    15252,    14693,
     14129,    13562,    12990,    12415,    11835,    11251,    10662,    10070,
     9473,    8872,    8266,    7657,    7043,    6424,    5802,    5175,
     4543,    3908,    3267,    2623,    1974,    1320,    662,    0

	};
	return pwmval[x>>8]>>4;
}


#define SCALE(x) ((x)*4+10)

#define WATERX SCALE(19)
#define WATERY SCALE(6)

#define DAMP 0.95
#define DROPSZ 1
#define DROPVAL 0x3ffff

int main(int argc, char **argv) {
	int x, y, i, xx, yy;
	int kbd;
	int leds[144];
	int buffa[WATERX*WATERY];
	int buffb[WATERX*WATERY];
	int *buff1=buffa, *buff2=buffb, *tmp;
	float damp=DAMP; //damping
	struct input_event ev;

	if (argc==1) {
		printf("Usage: %s /dev/input/eventX\n", argv[1]);
		exit(0);
	}

	for (x=0; x<WATERX*WATERY; x++) {
		buff1[x]=0;
		buff2[x]=0;
	}

	//Black out all LEDs
	for (x=0; x<140; x++) leds[x]=0xfff;

	//Open event device and USB device
	kbd=open(argv[1], O_RDONLY|O_NONBLOCK);
	usbDevOpen();

	while(1) {
		while (read(kbd, &ev, sizeof(ev))>0) {
			if (ev.type == EV_KEY && ev.value==1) {
				i=0;
				while(keys[i].id>=0) {
					if (keys[i].keycode==ev.code) {
						x=SCALE(keys[i].x);
						y=SCALE(keys[i].y);
						for (xx=x-DROPSZ; xx<x+DROPSZ; xx++) {
							for (yy=y-DROPSZ; yy<y+DROPSZ; yy++) {
								buff1[xx+yy*WATERX]=DROPVAL;
							}
						}
						printf("Key %d val %d id %d (%d,%d)\n", (int)ev.code, (int)ev.value, i, x, y);
					}
					i++;
				}
			}
		}


		//Simulate fluid
		for (x=1; x<WATERX-1; x++) {
			for (y=1; y<WATERY-1; y++) {
				buff2[x+y*WATERX]= \
					(float)((buff1[(x-1)+y*WATERX]+ \
					buff1[(x+1)+y*WATERX]+ \
					buff1[x+(y+1)*WATERX]+ \
					buff1[x+(y-1)*WATERX])/2 - buff2[x+y*WATERX])*damp;
			}
		}

		//Convert fluid to LED intensity
		i=0;
		while(keys[i].id!=-1) {
			x=SCALE(keys[i].x);
			y=SCALE(keys[i].y);
			x=buff1[x+y*WATERX];
			if (x<0) x=-x;
			if (x>0xffff) x=0xffff;
			leds[keys[i].id]=correctpwm(x);
			i++;
		}

		//Send LEDs and sleep
		sendLeds(leds, 0, 144);
		usleep(10000);
		//Swap buffers
		tmp=buff1;
		buff1=buff2;
		buff2=tmp;
	}

	usbDevClose();
	return 0;
}