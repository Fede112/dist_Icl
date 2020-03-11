#include <fstream>
#include <vector>
#include <iostream>
#include "cxxopts.hpp"

// Small Clustered Alignment structure (only essential data...)
struct SmallCA
{
    unsigned int qID;
    unsigned int sID;
    unsigned short sstart;
    unsigned short send;
    
    SmallCA(unsigned int q, unsigned int s, unsigned short ss, unsigned short se): qID(q), sID(s), sstart(ss), send (se) {}

};


//print a ClusteredAlignment TO STDOUT
void printSCA( SmallCA SCA) {

    std::cout << SCA.qID << " " << SCA.sID << " " << SCA.sstart << " " << SCA.send << std::endl;
    
}



int main(int argc, char** argv)
{


    ////////////////////////////////////////////////////////////////////////
    // Args parser (I tried a new library but I am not convinced)

    cxxopts::Options options("txt2binary", 
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
    char * buffer;
    unsigned int * p;
    unsigned short * pos ;
    SmallCA * sca;
    if (infile) 
    {
        // get length of file:
        std::cerr << "all characters read successfully. \n";
        infile.seekg (0, infile.end);
        length = infile.tellg();
        infile.seekg (0, infile.beg);

        buffer = new char [length];
        p = (unsigned int*) buffer;
        pos = (unsigned short*) buffer;
        sca = (SmallCA*) buffer;
        
        std::cerr << "Reading " << length << " characters... ";
        // read data as a block:
        infile.read (buffer,length);
    }
    else
    {
      std::cout << "error: only " << infile.gcount() << " could be read";
    }
    infile.close();
    
    unsigned long int bytes_line = 2*sizeof(unsigned int) + 2*sizeof(unsigned short);


    for (int i = 0; i < length/(bytes_line); ++i)
    {
        // pointer arithmetic depends on the pointer type: 3 unsigned int per line || 6 unsigned short per line.
        std::cout << p[3*i] << ' ' << p[3*i + 1] << ' ' << pos[6*i + 4] << ' ' << pos[6*i + 4 + 1] <<'\n';    
        // printSCA(sca[i]);


    }
    

    delete[] buffer;

  return 0;
}

