#!/bin/bash
# Modified rapidisnake script for v60Mini

fwupd=$1
sha1sum106="1b91d9e3c63e9c4b429651e44e9fce631b6b7e73"
sha1sum107="3d81f8f8d5821b14527e1f05a1a3d21276d623d1"

if [ ! -e "$fwupd" ]; then
	echo "I need firmware (cykb112_v107.exe) as first argument. Please download"
	echo "it from official KBParadise V60 facebook page and re-run this."
	exit 0
fi

echo "Building firmware modification tool..."
if ! make -C rapidisnake/rapiditool; then
	echo "Build failed! Please check if all dependencies (libusb1.0-dev, make, gcc) are installed."
	exit 0
fi

echo "Found usable $fwupd."
echo "Checking SHA1 for $fwupd."
echo "3d81f8f8d5821b14527e1f05a1a3d21276d623d1 cykb112_v107.exe" | sha1sum -c - || exit 1
echo "Extracting obfuscated firmware..."
mkdir "hack"
dd status=none if="$fwupd" of="hack/$fwupd.bin.enc" bs=$((0x054000)) skip=1
echo "De-obfuscating firmware..."
gcc -o dec dec.c
./dec da 62 82 cd < hack/$fwupd.bin.enc > hack/$fwupd.dec.bin 

# TODO: what to do with the decrypted file?

echo "All done!"
