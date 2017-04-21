#!/bin/bash
# Modified rapidisnake script for v60Mini
# This script patches the v60Mini updater to allow flash reading from pok3rtool

fwupd=$1
sha1sum107="3d81f8f8d5821b14527e1f05a1a3d21276d623d1"
patched='49e535ca91b325564c0bfc174af4f2293c3066e6'

if [ ! -e "$fwupd" ]; then
	echo "I need updater (cykb112_v107.exe) as first argument. Please download"
	echo "it from official KBParadise V60 facebook page and re-run this."
	exit 1
fi

echo "Found usable $fwupd."
echo "Checking SHA1 for $fwupd."
echo "$sha1sum107 $fwupd" | sha1sum -c - || exit 1

echo "Making copy to edit..."
mkdir output
cp "$fwupd" output/"$fwupd"_patched.exe

echo "Patching updater..."
# Use dd to overwrite first 4 bytes that disable flash dumping
# Check the docs for more info
printf '\x0a\x60\x90\x58' | dd conv=notrunc of=output/"$fwupd"_patched.exe bs=1 seek=$((0x00055ed0))
echo "Done. Checking SHA1..."
echo "$patched" output/"$fwupd"_patched.exe | sha1sum -c - || exit 1
echo "All done! Use the patched updater on Windows to unlock the flash."
