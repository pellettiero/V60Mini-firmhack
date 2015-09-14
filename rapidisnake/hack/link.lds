/*
Modified from another Cortex-M3 linker file by Sprite_tm <jeroen@spritesmods.com>. 
Original copyright notice follows:
*/

/*
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2010, Roel Verdult
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* Usable memory FOR THE APP! This does NOT describe all the memory in the controller. */
MEMORY
{
  ROM (rx) : ORIGIN = 0x00007000, LENGTH = 0x0009000
  RAM (rw) : ORIGIN = 0x20001800, LENGTH = 0x00000800
}


ENTRY(boot_entry)

SECTIONS
{
  .text :
  {
	hooks.o(.text)
    *(.text*)
    *(.rodata*)
  } > ROM

  _etext = .;
  .data : 
  {
    _data = .;
    *(vtable)
    *(.data*)
    _edata = .;
  } > RAM

  /* zero initialized data */
  .bss :
  {
    _bss = .;
    *(.bss*)
    *(COMMON)
    _ebss = .;
  } > RAM
  
  end = .;

}

/* Addresses of stuff in flash, for firmware 1.1.7 */
keydownorg = ABSOLUTE(0x2e38);
tickorg = ABSOLUTE(0x60b8);
initorg = ABSOLUTE(0x2d50);
flashorg = ABSOLUTE(0x307D);


ledIntensity = ABSOLUTE(0x20000490);
/*keyBitmap = ABSOLUTE(0x2000045C);*/
keyBitmap = ABSOLUTE(0x2000044a);
lightMode = ABSOLUTE(0x20000050);


