#include <fstream>
#include <iostream>
#include <unistd.h> // getopt
#include <vector>

#include "datatypes.h"

int main(int argc, char** argv)
{
    //-------------------------------------------------------------------------
    // Argument parser

    int opt;
    std::string inFilename, outFilename;

    while ((opt = getopt(argc, argv, "ho:")) != -1) 
    {
        switch (opt) 
        {
        case 'o':
            outFilename = optarg;
            break;
        case 'h':
            // go to default
            std::cout << "Reads dist_Icl binary output file and modifies distance to be 1-distance." << std::endl;
            std::cout << "Usage: " << argv[0] << " file.bin" << std::endl;
            break;

        default: /* '?' */
            std::cout << "Usage: " << argv[0] << " file.bin" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    if (optind != argc - 1) 
    {
        std::cerr << "Expected single argument after options." << std::endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        inFilename = argv[optind];
        std::cerr << "Input: " << inFilename << std::endl;
    }

    //-------------------------------------------------------------------------


    std::ifstream infile (inFilename, std::ifstream::binary);
    size_t bufferLen = 0;
    char * buffer = NULL;
    NormalizedPair * pPair = NULL;

    // read file
    if (infile) 
    {
        // get bufferLen of file:
        infile.seekg (0, infile.end);
        bufferLen = infile.tellg();
        infile.seekg (0, infile.beg);

        buffer = new char [bufferLen];
        
        std::cerr << "Reading " << bufferLen << " characters... ";
        // read data as a block:
        infile.read (buffer,bufferLen);
        std::cerr << "all characters read successfully. \n";
        
        pPair = (NormalizedPair*) buffer;
    }
    else
    {
      std::cout << "error: only " << infile.gcount() << " could be read \n";
    }
    infile.close();
    
    // each line has 3 unsigned int entries
    size_t lines = bufferLen/(sizeof(NormalizedPair));

    for (size_t i = 0; i < lines; ++i)
    {
    	pPair->distance = 1.-pPair->distance;
    	pPair++;
    }

    // std::cout << "Writing to " << output << "... ";
    auto outFile = std::fstream(outFilename, std::ios::out | std::ios::binary);
    outFile.write(buffer, bufferLen);
    return 0;
}
