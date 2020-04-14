#include <fstream>
#include <vector>
#include <iostream>

#include "smallca.h"
#include "cxxopts.hpp"

int main(int argc, char** argv)
{


    ////////////////////////////////////////////////////////////////////////
    // Args parser (I tried a new library but I am not convinced)

    cxxopts::Options options("Reads binary output file", 
        "Reads dist_Icl binary output file and outputs it to terminal.\n\n"
        "Output has 3 columns q*100+c, s, ss, se.\n");

    std::string input;
    options.add_options()
        ("h,help", "Print usage")
        ("i,input", "Input file path", cxxopts::value<std::string>())
    ;

    
    auto args = options.parse(argc, argv);


    if (args.count("help"))
    {
      std::cout << options.help() << std::endl;
      exit(0);
    }
    if (args.count("input")==1)
        input = args["input"].as<std::string>();
    else
    {
        std::cout << "Missing input flag: -i <filepath>" << std::endl;
        exit(1);
    }
    ////////////////////////////////////////////////////////////////////////


    
    std::ifstream infile (input, std::ifstream::binary);
    uint64_t length = 0;
    char * buffer = NULL;
    uint32_t * pInteger = NULL;
    double * pDouble = NULL;
    if (infile) 
    {
        // get length of file:
        infile.seekg (0, infile.end);
        length = infile.tellg();
        infile.seekg (0, infile.beg);

        buffer = new char [length];
        // p = (unsigned int*) buffer;
        // pos = (unsigned short*) buffer;
        
        std::cerr << "Reading " << length << " characters... ";
        // read data as a block:
        infile.read (buffer,length);
        std::cerr << "all characters read successfully. \n";
        
        pInteger = (unsigned int*) buffer;
        pDouble = (double *) buffer;
    }
    else
    {
      std::cout << "error: only " << infile.gcount() << " could be read";
    }
    infile.close();
    
    // each line has 3 unsigned int entries
    uint32_t lines = length/(2*sizeof(uint32_t) + sizeof(double));
    std::cerr << lines << std::endl;

    for (uint64_t i = 0; i < lines; ++i)
    {
        // pointer arithmetic depends on the pointer type: 3 unsigned int per line || 6 unsigned short per line.
        std::cout << pInteger[4*i+1] << ' ' << pInteger[4*i] << ' ' << pDouble[2*i + 1] <<'\n';    
    }
    

    delete[] buffer;

  return 0;
}

