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
	./$(EXE) ./data/BIG1_10e5.bin  ./data/BIG2_10e5.bin 0 0.02
# this is a minimsl run (parameter 0.01 means 0.01 hours) that starts from the recovery id 0 (i.e. from the very beginnig of the file)


debug: CXXFLAGS += $(DEBUG)
debug: default
debug: 
	valgrind -v ./${EXE} ./data/BIG1_10e4.bin  ./data/BIG2_10e4.bin 0 0.02

# TESTS:

test: output

output:
	head -30 ./outfile > ./test/a.txt
	tail -30 ./outfile > ./test/b.txt
	cmp ./test/a.txt ./test/reference/out_head.txt || exit 1
	cmp ./test/b.txt ./test/reference/out_tail.txt || exit 1
	rm -f ./test/a.txt ./test/b.txt
	@echo "Output test passed!"

.PHONY: default clean aux run debug test