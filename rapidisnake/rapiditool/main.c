/*
Tool to:
- Get flash dump from keyboard with modified firmware
- Decode fw from fw update exe to plain bytes that end up in firmware
- Encode plain bytes and use that as a firmware update
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

//Fix crc of a packet
void fixcrc(char *buff, int len) {
	int c;
	setword(&buff[2], 0);
	c=crc16(buff, len);
	setword(&buff[2], c);
}

//Part of the firmware is byte-order-messed-with and then  xor'ed with these values.
//This array can be found at 0x2348 in the flash of the chip.
const char xorData[]={
	0xAA,0x55,0xAA,0x55,0x55,0xAA,0x55,0xAA,0xFF,0,
	0,0,0,0xFF,0,0,0,0,0xFF, 0,
	0,0,0,0xFF,0,0,0,0,0xFF,0xFF,
	0xFF,0xFF,0xF,0xF,0xF,0xF,0xF0,0xF0,0xF0,0xF0,
	0xAA,0xAA,0xAA,0xAA,0x55,0x55,0x55,0x55,0,0,
	0,0
};

/*
A bit of terminology: the firmware is sent 'encrypted' (by xorring and byteswapping) to the keyboard
in the 10th till 100th packet. I call the process of encryption 'frizzling' and the process the keyboard
does to get the plain bytes out again 'defrizzling'. The 'frizzle counter' is basically a counter
that counts how many flash packets have been received.

And yes, terminology like that is what you sometimes get when staring at disassemblies for too long.
*/

//Per frizzle counter increase, this indicates how the bytes are swapped around.
//0,1,2,3 is not swapped.
const char frizzleOrder[]={
	2,1,3,0,
	3,2,1,0,
	3,1,0,2,
	1,2,0,3,
	2,3,1,0,
	0,2,1,3,
	0,1,2,3,
	1,2,3,0
};

//Undo the byte-swapping bit of the frizzling
void defrizzleOrder(char *buff, int frc) {
	char org[4];
	int x;
	int b=(frc&0x7)<<2;
	org[0]=buff[0]; org[1]=buff[1]; org[2]=buff[2]; org[3]=buff[3];
	buff[0]=org[frizzleOrder[b++]];
	buff[1]=org[frizzleOrder[b++]];
	buff[2]=org[frizzleOrder[b++]];
	buff[3]=org[frizzleOrder[b++]];
}

//Do the byte-swapping bit of the frizzling
void refrizzleOrder(char *buff, int frc) {
	char org[4];
	int x;
	int b=(frc&0x7)<<2;
	org[0]=buff[0]; org[1]=buff[1]; org[2]=buff[2]; org[3]=buff[3];
	buff[frizzleOrder[b++]]=org[0];
	buff[frizzleOrder[b++]]=org[1];
	buff[frizzleOrder[b++]]=org[2];
	buff[frizzleOrder[b++]]=org[3];
}

void defrizzleBuffer(char *buff, int frc) {
	int y;
	//First, xor with stored decryption key
	for (y=0; y<0x34; y++) {
		buff[y]^=xorData[y];
	}
	//Secondly, un-mix-up the bytes in each word.
	for (y=0; y<0x34; y+=4) defrizzleOrder(&buff[y], frc-0xa);
}

void frizzleBuffer(char *buff, int frc) {
	int y;
	//First, mix up the bytes in each word.
	for (y=0; y<0x34; y+=4) refrizzleOrder(&buff[y], frc-0xa);
	//Second, xor with stored decryption key
	for (y=0; y<0x34; y++) {
		buff[y]^=xorData[y];
	}
}

void decryptFwUpdate(int in, int out) {
	int x, y, frc;
	int l1, l2;
	char buff1[0x400];
	char buff2[0x400];
	char mem[65535];
	int mp=0;
	int p=0;

	read(in, buff1, 5); //skip header
	//Read the file, xoring and reversing the blocks where needed. This step is normally done
	//in the firmware update executable.
	while((l1=read(in, buff1, 0x400))!=0) {
		l2=read(in, buff2, 0x400);
		for (x=0; x<0x400; x++) {
			buff1[x]^=165;
			buff2[x]^=165;
		}
		if (p<0x3800) {
			memcpy(&mem[mp], buff2, l2); mp+=l2;
			memcpy(&mem[mp], buff1, l1); mp+=l1;
		} else {
			memcpy(&mem[mp], buff1, l1); mp+=l1;
			memcpy(&mem[mp], buff2, l2); mp+=l2;
		}
		p+=0x800;
	}
	
	//De-frizzle the scrambled bits of the firmware to get the image that ends up
	//in flash. This is normally done by the flash writing routines inside the keyboard controller.
	frc=0x0a; //frizzle counter
	for (x=0x34*0xa; x<=0x34*0x64; x+=0x34) {
		defrizzleBuffer(&mem[x], frc);
		frc++;
	}
	
	//All done.
	write(out, mem, mp);
}

//Quick-and-dirty hexdump routine
void dump(char *b, int len) {
	int x, y;
	for (x=0; x<len; x++) {
		if (x!=0 && (x&15)==0) {
			for (y=-15; y!=1; y++) {
				printf("%c", ((b[x+y]<32 || b[x+y]>127)?'.':b[x+y]));
			}
			printf("\n");
		}
		if ((x&3)==0) printf(" ");
		printf("%02hhx ", b[x]);
	}
	printf("\n");
}

//Cmd 1 subcmd 2: read memory
int getmem(char *buff, int a) {
	int l;
	memset(buff, 0, 64);
	buff[0]=1;
	buff[1]=2;
	setint(&buff[4], a);
	setint(&buff[8], a+0x3f);
	fixcrc(buff, 64);
	if (libusb_interrupt_transfer(dev, 0x04, buff, 64, &l, 1000)!=0) perror("int xfer w");
	if (libusb_interrupt_transfer(dev, 0x83, buff, 64, &l, 100)!=0) perror("int xfer r");
	return l;
}

//Cmd 3 subcmd 0: dunno what this does.
void reqDunno() {
	int l;
	char buff[64];
	memset(buff, 0, 64);
	buff[0]=3;
	buff[1]=0;
	fixcrc(buff, 64);
	if (libusb_interrupt_transfer(dev, 0x04, buff, 64, &l, 1000)!=0) perror("int xfer w");
	if (libusb_interrupt_transfer(dev, 0x83, buff, 64, &l, 100)!=0) perror("int xfer r");
	return;
}

//Cmd 4 subcmd 0/1: enable/disable flash mode
void enaFlash(int ena) {
	int l;
	char buff[64];
	memset(buff, 0, 64);
	buff[0]=4;
	buff[1]=ena;
	fixcrc(buff, 64);
	if (libusb_interrupt_transfer(dev, 0x04, buff, 64, &l, 1000)!=0) perror("int xfer w");
	return;
}

//Cmd 0 subcmd 8: erase flash
void eraseFlash(int from, int to) {
	int l;
	char buff[64];
	memset(buff, 0, 64);
	buff[0]=0;
	buff[1]=8;
	setint(&buff[4], from);
	setint(&buff[8], to);
	fixcrc(buff, 64);
	if (libusb_interrupt_transfer(dev, 0x04, buff, 64, &l, 1000)!=0) perror("int xfer w");
}

//Cmd 1 subcmd 1/0: flash/verify bytes
void flashBytes(int loc, int len, char *data, int isVerify) {
	int l;
	char buff[64];
	if (len>0x34) {
		printf("BUG! flashBytes len %x > 0x34\n");
		return;
	}

	memset(buff, 0, 64);
	buff[0]=1;
	buff[1]=isVerify?0:1;
	setint(&buff[4], loc);
	setint(&buff[8], loc+len-1);
	memcpy(&buff[12], data, len);
	fixcrc(buff, 64);
//	dump(buff, 64);
	if (libusb_interrupt_transfer(dev, 0x04, buff, 64, &l, 1000)!=0) perror("int xfer w");
}

//Set version to something else
void resetVer(char *ver) {
	char buff[64];
	memset(buff, 0, 64);
	setint(&buff[0], 5);
	memcpy(&buff[4], ver, 5);
	eraseFlash(0x2800, 0x280b);
	flashBytes(0x2800, 9, buff, 0);
}

//Try and detach the USB endpoints we need from the HID driver.
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

//Open and detach an USB device
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

//Try and open an usb connection to the keyboard somehow.
void usbDevOpen() {
	libusb_init(&ctx);

	dev=usbOpenAndDetach(0x2516, 0x0020, 1);
	if (dev==NULL) {
		//Curious. Maybe the device is in flash mode.
		dev=usbOpenAndDetach(0x2516, 0x0023, 1);
		if (dev==NULL) {
			printf("opening device: none found\n");
			exit(0);
		} else {
			fprintf(stderr, "Warning: Device already is in flash mode!\n");
			devInFlashMode=1;
		}
	} else {
		devInFlashMode=0;
	}

	if (libusb_set_configuration(dev, 1)!=0) {
		//Don't complain; probably already set by usbhid
		//perror("setting config iface");
	}
	if (libusb_claim_interface(dev, 1)!=0) perror("claim iface");
}

//Make the keyboard go to flash mode.
void usbDevGotoFlashMode() {
	int x;
	if (devInFlashMode) return;
	fprintf(stderr, "Enabling flash mode...\n");

	enaFlash(1);
	sleep(1);
	libusb_close(dev);
	//Try to open flash dev for 10 secs.
	for (x=0; x<100; x++) {
		dev=usbOpenAndDetach(0x2516, 0x0023, 1);
		if (dev!=NULL) break;
		usleep(100*1000);
	}
	if (dev==NULL) {
		perror("opening device");
		exit(1);
	}
	if (libusb_set_configuration(dev, 1)!=0) {
		//Don't complain; probably already set by usbhid
		//perror("setting config iface");
	}
	if (libusb_claim_interface(dev, 1)!=0) perror("claim iface");
	fprintf(stderr, "Enabled.\n");
}

//Close the USB dev
void usbDevClose() {
	//libusb_attach_kernel_driver(dev, 1);
	libusb_close(dev);
	libusb_exit(ctx);
}

#define ACTION_NONE 0
#define ACTION_GETVER 1
#define ACTION_SETVER 2
#define ACTION_READFLASH 3
#define ACTION_WRITEAPP 4
#define ACTION_DECRYPT 5
#define ACTION_FN5 6

int main(int argc, char **argv) {
	char buff[64];
	int r, l, x;
	int skipFlashModeEna=0;
	int action=ACTION_NONE;

	if (argc>1) {
		if (strcmp(argv[1], "-getver")==0) {
			action=ACTION_GETVER;
		} else if (strcmp(argv[1], "-setver")==0 && argc>2) {
			action=ACTION_SETVER;
		} else if (strcmp(argv[1], "-readflash")==0 && argc>2) {
			action=ACTION_READFLASH;
		} else if (strcmp(argv[1], "-writeapp")==0 && argc>2) {
			action=ACTION_WRITEAPP;
		} else if (strcmp(argv[1], "-decr")==0 && argc>3) {
			action=ACTION_DECRYPT;
		} else if (strcmp(argv[1], "-fn5")==0) {
			action=ACTION_FN5;
		}
	}

	if (action==ACTION_NONE) {
		printf("App to interrogate the firmware of a Coolermaster Rapid I\n");
		printf("Usage: %s cmd [file|str]\n", argv[0]);
		printf(" -getver: Read the firmware version\n");
		printf(" -setver 1.2.3: Change the reported firmware version\n");
		printf(" -readflash out.bin: Try to dump flash. Needs modified firmware\n");
		printf(" -writeapp app.bin: Write new fw app\n");
		printf(" -decr fwupd.bin app.bin: Convert firmware update image to flash contents\n");
		printf(" -fn5: Test fn5 (sed led intensity) functionality of hack\n");
		exit(0);
	}

	if (action==ACTION_GETVER) {
		usbDevOpen();
		getmem(buff, 0x2800);
		dump(buff, 64);
		usbDevClose();
	}

	if (action==ACTION_SETVER) {
		usbDevOpen();
		usbDevGotoFlashMode();
		resetVer(argv[2]);
		getmem(buff, 0x2800);
		dump(buff, 10);
		usbDevClose();
	}

	if (action==ACTION_DECRYPT) {
		int in, out;
		in=open(argv[2], O_RDONLY);
		if (in<=0) {
			perror(argv[1]);
			exit(1);
		}
		out=open(argv[3], O_WRONLY|O_TRUNC|O_CREAT, 0644);
		if (out<=0) {
			perror(argv[1]);
			exit(1);
		}
		decryptFwUpdate(in, out);
		close(in);
		close(out);
	}

	if (action==ACTION_READFLASH) {
		int f;
		f=open(argv[2], O_WRONLY|O_TRUNC|O_CREAT, 0644);
		if (f<=0) {
			perror(argv[1]);
			exit(1);
		}
		usbDevOpen();
		for (x=0; x<0x20000; x+=64) {
			getmem(buff, x);
			write(f, buff, 64);
		}
		close(f);
		usbDevClose();
	}

	if (action==ACTION_WRITEAPP) {
		int f, p, n, e, xt, i;
		int frc=0;
		size_t flen;
		usbDevOpen();
		usbDevGotoFlashMode();
		fprintf(stderr, "Writing app.\n");
		f=open(argv[2], O_RDONLY);
		if (f<=0) {
			perror(argv[1]);
			exit(1);
		}
		reqDunno(); //No idea what this returns. It does reset the frizzle counter tho'.

		eraseFlash(0x2800, 0x280b);
		flen=lseek(f, 0, SEEK_END);
		printf("Erasing from %x to %x...\n", 0x2c00, 0x2c00+flen);
		eraseFlash(0x2c00, 0x2c00+flen);

		printf("Writing data...\n");
		for (n=0; n<2; n++) {
			lseek(f, 0, SEEK_SET);
			p=0x2c00;
			e=0x2c00;
			while ((l=read(f, buff, 0x34))!=0) {
				if (frc>=0xa && frc<=0x64) {
					//Bytes in this region need to be sent scrambled.
					frizzleBuffer(buff, frc);
				}
//				printf("%p\n", p);
				flashBytes(p, l, buff, n);
				p+=l;
				frc++;
			}
		}

		memset(buff, 0, 64);
		setint(&buff[0], 5);
		memcpy(&buff[4], "1.1.7", 5); //Hardcoded... Should read this from a file or something.
		flashBytes(0x2800, 9, buff, 0);
		sleep(1);
		enaFlash(0);

		close(f);
	}
	return 0;
}
