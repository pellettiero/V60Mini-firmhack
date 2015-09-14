CROSSPREFIX=arm-none-eabi-
CC=$(CROSSPREFIX)gcc
LD=$(CROSSPREFIX)ld
OBJDUMP=$(CROSSPREFIX)objdump
OBJCOPY=$(CROSSPREFIX)objcopy
CFLAGS=-mthumb -mcpu=cortex-m3 -Os
LDFLAGS=-T link.lds
OBJS=main.o snake.o hooks.o startup.o
ORGFWUPD=orgfwupd.bin
OUT=hackedfw.bin

all: $(OUT)

mycode.elf: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

mycode.bin: mycode.elf
	$(OBJCOPY) -O binary mycode.elf mycode.bin

$(OUT): mycode.bin $(ORGFWUPD) Makefile
	cp -f $(ORGFWUPD) $(OUT).tmp
#Patch in start hook
	./patch.sh $(OUT).tmp -set $$((0x2c04-0x2c00)) 09700000
#Patch in keypress hook
	./patch.sh $(OUT).tmp -set $$((0x44be-0x2c00)) 02f09ffd
#Patch in idle tick hook
	./patch.sh $(OUT).tmp -set $$((0x6652-0x2c00)) 00f0d7fc
#Patch in USB function 0x4 (flash) handler
	./patch.sh $(OUT).tmp -set $$((0x6934-0x2c00)) 11700000
#Patch in USB function 0x5 handler
	./patch.sh $(OUT).tmp -set $$((0x6938-0x2c00)) 0d700000
#Patch out range check on flash read USB command
	./patch.sh $(OUT).tmp -set $$((0x61F2-0x2c00)) 00bf
#Add the code
	./patch.sh $(OUT).tmp -add $$((0x7000-0x2c00)) mycode.bin
#All done!
	mv -f $(OUT).tmp $(OUT)

flash: $(OUT)
	../rapiditool/rapiditool -writeapp $(OUT)


clean:
	rm -f $(OUT) mycode.bin mycode.eld $(OBJS)
