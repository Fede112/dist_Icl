# Readme

This is a collection of c++ programs to compute the distance between primary clusters (see ref..).

The main computation is perfomed by the program dist_Icl.x : this program compute the distances between clustered sequences contained in tue different files, input1 and input2. These two files must not contain both sequences of the same cluster. If you want to compiute distances between clusters contained in the same input file, another version of the program is needed (todo). 

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
The data need to be prepared by cleaning and sorting it. The basic command to do so would be:
awk '$2>=0' DATA_TOCELAN | sort -gk3 > DATA_CLEANED
(note that this pipeline do not suit huge files as those we are working with.)

DATA_CLEANED files contain the following columns:
q_id cl_relativeID s_id s_start s_end



### TODO
- Include normalization (clI sizes needed, but they are easy to compute)
- dist_Icl VARIANT to anlyze the same infile with itself

