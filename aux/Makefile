# -*- Makefile -*-
SHELL=/bin/sh

CXX=g++
# CFLAGS= -Wall -Wextra -O3 -ffast-math -fexpensive-optimizations -msse3 
CXXFLAGS= -O3 -std=c++11 -I ../include/ -Wall
LDFLAGS= # -lfmt
DEBUG= -g -ggdb


SRC=read_input.cc read_output.cc input2binary.cc 
EXE=$(SRC:.cc=.x)

default: $(EXE)

%.x:%.cc
	$(CXX) $(CXXFLAGS) $< -o $@

# debug: CXXFLAGS += $(DEBUG)
# debug: default
# debug: 
# 	valgrind ./${EXE}

clean:
	rm -rf *~ $(EXE)
