#!/bin/bash
# Modified rapidisnake script for v60Mini

fwupd=$1
sha1sum107="3d81f8f8d5821b14527e1f05a1a3d21276d623d1"

if [ ! -e "$fwupd" ]; then
	echo "I need updater (cykb112_v107.exe) as first argument. Please download"
	echo "it from official KBParadise V60 facebook page and re-run this."
	exit 1
fi

echo "Found usable $fwupd."
echo "Checking SHA1 for $fwupd."
echo "$sha1sum107 $fwupd" | sha1sum -c - || exit 1
echo "Extracting obfuscated updater firmware..."
mkdir output
dd status=none if="$fwupd" of="output/$fwupd.bin.enc" bs=$((0x054000)) skip=1
echo "Compiling decryptor..."
gcc -o dec dec.c || exit 1
echo "De-obfuscating updater firmware..."
./dec da 62 82 cd < output/"$fwupd".bin.enc > output/"$fwupd".dec.bin

echo "Updater firmware ($fwupd.dec.bin) decrypted succesfully."
echo "You can find it in /output"
