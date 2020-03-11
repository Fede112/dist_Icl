# -*- Makefile -*-
SHELL=/bin/sh

CXX=g++
# CFLAGS= -Wall -Wextra -O3 -ffast-math -fexpensive-optimizations -msse3 
CXXFLAGS= -O3 -std=c++11 -I ./include/# -Wall
LDFLAGS= # -lfmt
DEBUG= -g -ggdb


SRC=dist_Icl.cc txt2binary.cc binary2txt.cc
EXE=$(SRC:.cc=.x)

default:$(EXE)


%.x:%.cc
	$(CXX) $(CXXFLAGS) $< -o $@


clean:
	rm -rf *~ $(EXE) *.x outfile


run:$(EXE)
	./$(EXE) ./data/BIG1_map2.clean  ./data/BIG2_map2.clean 0 0.02
# this is a minimsl run (parameter 0.01 means 0.01 hours) that starts from the recovery id 0 (i.e. from the very beginnig of the file)



# TESTS:

test: output

output:
	head -30 ./outfile > ./test/a.txt
	tail -30 ./outfile > ./test/b.txt
	cmp ./test/a.txt ./test/reference/out_head.txt || exit 1
	cmp ./test/b.txt ./test/reference/out_tail.txt || exit 1
	rm -f ./test/a.txt ./test/b.txt
	@echo "Output test passed!"

.PHONY: default clean test run