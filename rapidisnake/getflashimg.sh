#!/bin/bash

fwupd="Rapid i Firmware Pack _v117_US.exe"
md5="eebafbfde451577f27d7f86d791725e9"

if [ ! -e "$fwupd" ]; then
	echo "I need $fwupd to work. Please download"
	echo "it from http://gaming.coolermaster.com/en/products/keyboards/rapid-i/ and"
	echo "re-run this."
	exit 0
fi

cmd5=`md5sum "$fwupd" | cut -d ' ' -f 1`
if [ "$md5" != "$cmd5" ]; then
	echo "Sorry, md5sum from $fwupd doesn't match, I can't safely use this."
	exit 0
fi

echo "Building firmware modification tool..."
if ! make -C rapiditool; then
	echo "Build failed! Please check if all dependencies (libusb1.0-dev, make, gcc) are installed."
	exit 0
fi
echo "Found usable $fwupd."
echo "Extracting obfuscated firmware..."
dd status=none if="$fwupd" of="hack/orgfwupd.bin.enc" bs=$((0x2bd400)) skip=1
echo "De-obfuscating firmware..."
./rapiditool/rapiditool -decr "hack/orgfwupd.bin.enc" "hack/orgfwupd.bin"

echo "All done!"
