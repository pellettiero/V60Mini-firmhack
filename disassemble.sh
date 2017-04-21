#!/bin/bash

# Script to disassemble an ARM binary
# From Charlie on Github: https://github.com/ChaoticConundrum
# You need the ARM objdump to do this
# Usage: disassemble.sh input_file output_file [vma_adjust]
arm-none-eabi-objdump -D -EL -b binary -m arm -M force-thumb -z --adjust-vma="$3" "$1" > "$2"

