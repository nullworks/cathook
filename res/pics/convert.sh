#!/usr/bin/env bash
convert "$1".png -depth 8 -format rgba "$1".rgba
mv "$1".rgba "$1"
objcopy --input binary --output elf32-i386 --binary-architecture i386 "$1" "$1".o
mv "$1".o bin/"$1".o
