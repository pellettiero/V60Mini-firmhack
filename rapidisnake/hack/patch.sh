#!/bin/bash

function doSet() {
	printf $(echo "$3" | sed -r 's/([0-9a-f]{2})/\\x\1/g') | dd status=none of="$1" bs=1 seek=$(($2)) conv=notrunc 
	#'
}

function doAdd() {
	dd status=none if="$3" of="$1" bs=1 seek=$(($2)) conv=notrunc
}

infile="$1"
if [ "$2" = "-set" ]; then
	doSet "$infile" "$3" "$4"
elif [ "$2" = "-add" ]; then
	doAdd "$infile" "$3" "$4"
else
	echo "$0 - small tool to patch binary files"
	echo "Usage:"
	echo "$0 infile.bin -set 0xoffset 1234abcd: sets the bytes at offset to the specified value. Size is dependent on amount of bytes given."
	echo "$0 infile.bin -add 0xoffset file.bin: modifies infile.bin to include the contents of file.bin at the specified offset"
fi

