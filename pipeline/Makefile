# -*- Makefile -*-
SHELL=/bin/sh

CXX=g++
# CXXFLAGS= -Wall -Wextra -O3 -ffast-math -fexpensive-optimizations -msse3 
CXXFLAGS= -O3 -std=c++14 -I ../include/ -Wall
LDFLAGS= -lpthread
DEBUG= -g -ggdb

EXE=pipeline.x
OBJS=pipeline.o ../src/normalization.o ../src/ascii2binary.o

default: $(EXE)


$(EXE): $(OBJS)
	$(CXX) $^ -o $@  $(LDFLAGS)

%.o: %.cc
	$(CXX) -c $< -o $@  $(CXXFLAGS) 


pipeline.o: ../include/smallca.h ../include/normalization.h ../include/ascii2binary.h
../src/normalization.o: ../include/smallca.h ../include/normalization.h 


run:$(EXE)
# 	input is a txt sorted wrt qID. -o flag gives an output
	./$(EXE) ../data_qsize/BIG1_10e5_qID.clean

# this is a minimsl run (parameter 0.01 means 0.01 hours) that starts from the recovery id 0 (i.e. from the very beginnig of the file)



clean:
	rm -rf *~ $(OBJS) *.x

