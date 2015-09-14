
CFLAGS=`pkg-config --cflags libusb-1.0` -ggdb
LDFLAGS=`pkg-config --libs libusb-1.0`

all: rapiditool kbledd

kbledd: kbledd.o
	$(CC) $(LDFLAGS)-o $@ $^

rapiditool: main.o
	$(CC) $(LDFLAGS)-o $@ $^

clean:
	rm -f *.o rapiditool kbledd
