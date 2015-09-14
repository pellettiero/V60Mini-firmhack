#Bit of assembly to handle the hooked functions.

# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 42):
# Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
# this notice you can do whatever you want with this stuff. If we meet some day, 
# and you think this stuff is worth it, you can buy me a beer in return. 
# ----------------------------------------------------------------------------

.code 16
.global hooktick
.global hookkeydown
.global usbfn5
.global tickorg
.global keydownorg
.global initorg

_start:
#Hook table. Order is fairly random because I added entries as I went.
.align 2
	b dohookkeydown
.align 2
	b dohooktick
.align 2
	b doinit
.align 2
	b dousbfn5
.align 2
	b doflash

dohooktick:
	push {r0, lr}
	bl hooktick
	bl tickorg
	pop {r0, pc}

dohookkeydown:
	push {r0, lr}
	bl hookkeydown
	bl keydownorg
	pop {r0, pc}


doinit:
	bl boot_entry
	bl initorg
