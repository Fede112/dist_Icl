# -*- Makefile -*-
SHELL=/bin/sh

CXX=g++
# CFLAGS= -Wall -Wextra -O3 -ffast-math -fexpensive-optimizations -msse3 
CXXFLAGS= -O3 -std=c++11 -I ./include/ -Wall
LDFLAGS= # -lfmt
DEBUG= -g -ggdb


SRC=dist_Icl.cc 
EXE=$(SRC:.cc=.x)
EXE_AUX=$(SRC_AUX:.cc=.x)

default: $(EXE)


%.x:%.cc
	$(CXX) $(CXXFLAGS) $< -o $@


clean:
	rm -rf *~ $(EXE)
	$(MAKE) $(MFLAGS) clean  -C ./aux/ 

aux:
	$(MAKE) $(MFLAGS) -C ./aux/

run:$(EXE)
	./$(EXE) ./data/BIG1_10e5.bin  ./data/BIG2_10e5.bin 5 0.02
# this is a minimsl run (parameter 0.01 means 0.01 hours) that starts from the recovery id 0 (i.e. from the very beginnig of the file)


debug: CXXFLAGS += $(DEBUG)
debug: default
debug: 
	valgrind ./${EXE} ./data/BIG1_10e4.bin  ./data/BIG2_10e4.bin 0 0.02

# TESTS:

test: output

output: $(EXE) aux
	@echo "Output test..."
	./$(EXE) ./data/BIG1_10e3.bin  ./data/BIG2_10e3.bin 0 0.02
	./aux/read_output.x -i outfile.bin > ./test/out.txt
	cmp ./test/out.txt ./test/reference/out_10e3.ref || rm -f ./test/out.txt outfile.bin | exit 1
	@rm -f ./test/out.txt outfile.bin
	@echo "Output test passed!"

.PHONY: default clean aux run debug test