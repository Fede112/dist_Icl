# -*- Makefile -*-
SHELL=/bin/sh

CXX=g++
# CXXFLAGS= -Wall -Wextra -O3 -ffast-math -fexpensive-optimizations -msse3 
CXXFLAGS= -O3 -std=c++14 -I ./include/ -Wall
LDFLAGS= -lpthread
DEBUG= -g -ggdb

EXE=dist_Icl.x
OBJS=dist_Icl.o src/normalization.o

default: $(EXE)


$(EXE): $(OBJS)
	$(CXX) $^ -o $@  $(LDFLAGS)

%.o: %.cc
	$(CXX) -c $< -o $@  $(CXXFLAGS) 


dist_Icl.o: include/smallca.h include/normalization.h
src/normalization.o: include/smallca.h include/normalization.h


diagonal:CXXFLAGS+= -DDIAGONAL
diagonal: default


run:$(EXE)
	./$(EXE) ./data_qsize/BIG1_10e5.bin  ./data_qsize/BIG2_10e5.bin
# this is a minimsl run (parameter 0.01 means 0.01 hours) that starts from the recovery id 0 (i.e. from the very beginnig of the file)



clean:
	rm -rf *~ $(OBJS) *.x
	$(MAKE) $(MFLAGS) clean  -C ./aux/
	$(MAKE) $(MFLAGS) clean  -C ./pipeline/



aux:
	$(MAKE) $(MFLAGS) -C ./aux/

pipeline:
	$(MAKE) $(MFLAGS) -C ./pipeline/



debug: CXXFLAGS += $(DEBUG)
debug: default
debug: 
	valgrind ./${EXE} ./data_qsize/BIG1_10e4.bin  ./data_qsize/BIG2_10e4.bin

# TESTS:

check: output

output: $(EXE) aux
	./reference/output_test.sh

.PHONY: default clean aux pipeline run debug test