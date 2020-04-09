#include <fstream>
#include <vector>
#include <iostream>
#include "cxxopts.hpp"

#include "smallca.h"
#include "cxxopts.hpp"

int main(int argc, char** argv)
{


    ////////////////////////////////////////////////////////////////////////
    // Args parser (I tried a new library but I am not convinced)

    cxxopts::Options options("Read binary input", 
        "Reads dist_Icl binary input file and outputs it to terminal.\n\n"
        "Output has 4 columns q*100+c, s, ss, se.\n");

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
    unsigned long int length = 0;
    char * buffer = NULL;
    // unsigned int * p;
    // unsigned short * pos ;
    SmallCA * sca = NULL;
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
        
        sca = (SmallCA*) buffer;
    }
    else
    {
      std::cout << "error: only " << infile.gcount() << " could be read";
    }
    infile.close();
    
    unsigned long int lines = length/sizeof(SmallCA);



    for (unsigned long int i = 0; i < lines; ++i)
    {
        // pointer arithmetic depends on the pointer type: 3 unsigned int per line || 6 unsigned short per line.
        // std::cout << p[3*i] << ' ' << p[3*i + 1] << ' ' << pos[6*i + 4] << ' ' << pos[6*i + 4 + 1] <<'\n';    
        printSCA(sca[i]);


    }
    

    delete[] buffer;

  return 0;
}

