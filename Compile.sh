#!/bin/bash
make clean
sleep .2
make
sleep .2
mv EBOOT.PBP build/EBOOT.PBP
mv PARAM.SFO build/PARAM.SFO
mv main.prx build/main.prx
pspsh
#PPSSPPSDL ./build/EBOOT.PBP
