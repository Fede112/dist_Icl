#!/bin/bash

#compile
g++ -o exe dist_Icl.cc -O3 -lfmt

#run
# this is a minimsl run (parameter 0.01 means 0.01 hours) that starts from the recovery id 0 (i.e. from the very beginnig of the file)
./exe BIG1_test.clean  BIG2_test.clean 0 0.04 > outfile
