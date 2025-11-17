#!/bin/bash
echo compiling && dasm makrotrak.asm -Isrc -Iaudio -Ivisual -omakrotrak.nes -f3 -v2 -llisting.txt
# dasm -sromsym.txt will export symbol file
