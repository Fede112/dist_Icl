# Readme

This is a collection of c++ programs to compute the distance between primary clusters (see ref..).

The main computation is perfomed by the program dist_Icl.cc .
Auxiliary progarms are located in the aux folder. Their function is mainly to convert from/to binary input and output files.


## Requirements
- g++ 7 or higher

## Installation
Installation consists in compiling the programs by using the Makefiles.
- Use "make" in the root folder to compile dist_Icl.c
- Use "make aux" in the root folder to compile auxiliary programs.


## Input data
Input data is derived from the primary_cl.cc output contained in (... ref). 
The original primary_cl.cc output contains alignments from a number of sequences (about 4000).
  requires to be prepared by cleaning and sorting it. We first nee



### TODO
- Include normalization (clI sizes needed, but they are easy to compute)

