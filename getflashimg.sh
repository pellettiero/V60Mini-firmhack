#!/bin/bash
# Modified rapidisnake script for v60Mini

fwupd=$1

if [ ! -e "$fwupd" ]; then
	echo "I need firmware as first argument. Please download"
	echo "it from official V60 facebook page and re-run this."
	exit 0
fi

echo "Building firmware modification tool..."
if ! make -C rapidisnake/rapiditool; then
	echo "Build failed! Please check if all dependencies (libusb1.0-dev, make, gcc) are installed."
	exit 0
fi

echo "Found usable $fwupd."
echo "Extracting obfuscated firmware..."
mkdir "hack"
dd status=none if="$fwupd" of="hack/$fwupd.bin.enc" bs=$((0x054000)) skip=1

# TODO create de-obfuscator for v60Mini
# echo "De-obfuscating firmware..."
# ./rapidisnake/rapiditool/rapiditool -decr "hack/$fwupd.bin.enc" "hack/$fwupd.bin"

echo "All done!"
