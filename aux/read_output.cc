#include <fstream>
#include <vector>
#include <iostream>

#include "smallca.hpp"
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
    unsigned long int length = 0;
    char * buffer = NULL;
    unsigned int * p = NULL;
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
        
        p = (unsigned int*) buffer;
    }
    else
    {
      std::cout << "error: only " << infile.gcount() << " could be read";
    }
    infile.close();
    
    // each line has 3 unsigned int entries
    unsigned long int lines = length/(3*sizeof(unsigned int));
    std::cerr << lines << std::endl;

    for (unsigned long int i = 0; i < lines; ++i)
    {
        // pointer arithmetic depends on the pointer type: 3 unsigned int per line || 6 unsigned short per line.
        std::cout << p[3*i] << ' ' << p[3*i + 1] << ' ' << p[3*i + 2] <<'\n';    
    }
    

    delete[] buffer;

  return 0;
}
