#!/bin/bash
source /cvmfs/sft.cern.ch/lcg/views/LCG_100rc2/x86_64-centos7-gcc10-opt/setup.sh && \

rm -rf * && \
/afs/desy.de/user/d/dudarboh/FCAL/tb20/LUCAS_TB20/bin/Lucas -m ${2} && \
mv -f lucas.root ../output/${1}.root
