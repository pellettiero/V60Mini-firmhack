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
#include <unistd.h>


/*
Usage:
gcc -o dec dec.c
./dec k1 k2 k3 k4 < in > out
with k[1-4] being the 32-bit key.

Eg For 107:
./dec da 62 82 cd < fwfrom107.bin > fwfrom107.dec.bin
*/


int main(int argc, char *argv[]) {
	int n=0;
    int x=0;
	char buf[4], key[4];

	key[0]=strtol(argv[1], NULL, 16);
	key[1]=strtol(argv[2], NULL, 16);
	key[2]=strtol(argv[3], NULL, 16);
	key[3]=strtol(argv[4], NULL, 16);

	while(read(0, buf, 4)) {
		for (x=0; x<4; x++) buf[x]=buf[x]^key[x]^(n+x);
		write(1, buf, 4);
		n+=4;
	}
}
