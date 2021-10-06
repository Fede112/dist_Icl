# -*- Makefile -*-
SHELL=/bin/sh

CXX=g++
# CXXFLAGS= -Wall -Wextra -O3 -ffast-math -fexpensive-optimizations -msse3 
CXXFLAGS= -O3 -std=c++14 -I ../include/ -Wall
LDFLAGS=
DEBUG= -g -ggdb

EXE=pipeline.x
OBJS=pipeline.o ../src/normalization.o

default: $(EXE)


$(EXE): $(OBJS)
	$(CXX) $^ -o $@  $(LDFLAGS)

%.o: %.cc
	$(CXX) -c $< -o $@  $(CXXFLAGS) 

pipeline.o: ../include/datatypes.h ../include/normalization.h
../src/normalization.o: ../include/datatypes.h ../include/normalization.h 


run:$(EXE)
# 	input is a txt sorted wrt qID. -o flag gives an output
	./$(EXE) ../data_qsize/BIG1_10e5_qID.clean

clean:
	rm -rf *~ $(OBJS) *.x

