# -*- Makefile -*-
SHELL=/bin/sh

CXX=g++
CXXFLAGS= -O3 -std=c++14 -I ../include/ -Wall -Wextra
LDFLAGS= # -lfmt
DEBUG= -g -ggdb


SRC=read_input.cc read_output.cc input2binary.cc output2binary.cc kmerge_binary.cc transform_output.cc


# OBJS=$(SRC:.cc=.o)
EXE=$(SRC:.cc=.x)


default: $(EXE)

$(EXE): %.x : %.cc
	$(CXX) $^ -o $@  $(LDFLAGS) $(CXXFLAGS) 

kmerge_binary.o: CXXFLAGS+=-fopenmp
kmerge_binary.x: LDFLAGS+=-fopenmp
input2binary.x: ../src/normalization.o

# %.o: %.cc
# 	$(CXX) -c $< -o $@  $(CXXFLAGS) 


# debug: CXXFLAGS += $(DEBUG)
# debug: default
# debug: 
# 	valgrind ./${EXE}

clean:
	rm -rf *~  $(EXE) ../src/normalization.x 

.PHONY: default clean
