#!/bin/bash
onchange -v -p 250 './**/*.asm' -- sh -c 'echo compiling && dasm makrotrak.asm -Isrc -Iassets -omakrotrak.nes -f3 -v2 -E2 -llisting.txt && echo launching && cmd.exe /C start makrotrak.nes'
# dasm -sromsym.txt will export symbol file
# https://www.npmjs.com/package/onchange
