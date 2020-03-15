# -*- Makefile -*-
SHELL=/bin/sh

CC=g++
# CFLAGS= -Wall -Wextra -O3 -ffast-math -fexpensive-optimizations -msse3 -lmft
CFLAGS= -O3
LDFLAGS= 
DEBUG= -g -ggdb

EXE = dist_Icl.x
OBJS = dist_Icl.o	


default:$(EXE)

$(EXE): dist_Icl.o 
# $(EXE): $(OBJS)
	$(CC) $^ -o $@  $(LDFLAGS)
	@rm $^

%.o: %.cc
	$(CC) -c $< -o $@  $(CFLAGS) 




run:$(EXE)
	./$(EXE) ./data/BIG1_10e5.clean  ./data/BIG2_10e5.clean 0 0.02 > outfile
# this is a minimsl run (parameter 0.01 means 0.01 hours) that starts from the recovery id 0 (i.e. from the very beginnig of the file)

clean:
	rm -rf *~ $(EXE) *.x outfile $(OBJS)







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