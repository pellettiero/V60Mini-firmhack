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
#include <stdint.h>
#include <pcap/pcap.h>
#include <unistd.h>

//Quick and dirty tool to distill whatever's written to flash from the usbpcap file.
//Used when I reverse engineered the protocol; not needed anymore.
//Miscompiles on 64bit systems :/

//Small routine to do a hexdump
void dump(char *b, int len) {
	int x, y;
	for (x=0; x<len; x++) {
		if (x!=0 && (x&15)==0) {
			for (y=-15; y!=1; y++) {
				fprintf(stderr, "%c", ((b[x+y]<32 || b[x+y]>127)?'.':b[x+y]));
			}
			fprintf(stderr,"\n");
		}
		if ((x&3)==0) fprintf(stderr," ");
		fprintf(stderr,"%02hhx ", b[x]);
	}
	fprintf(stderr,"\n");
}


//Usage: ./grabfw < file.pcap > out.bin
int main() {
	struct pcap_file_header pghd;
	struct pcap_pkthdr phd;
	unsigned char buff[65535];
	unsigned char mem[1024*64];
	int l;
	int a;

	read(0, &pghd, sizeof(pghd));
	while(read(0, &phd, sizeof(phd))!=0) {
		l=phd.caplen;
		if (l>512) {
			fprintf(stderr, "Huh? Buff=%d\n", l);
			exit(1);
		}
		//Grab data
		read(0, buff, l);
//		dump(buff,64);
		if (buff[27]==1 && buff[28]==1) {
			//Memcpy the data to where it belongs in the memory image
			a=buff[31]+(buff[32]<<8);
			memcpy(&mem[a], &buff[39], 0x34);
		}
	}
	//Dump memory image to stdout
	write(1, mem, 65535);
}
