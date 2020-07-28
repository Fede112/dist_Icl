/*******************************************************************************
* Reads dist_Icl binary output and outputs it to terminal
* 
* The output consists of 3 columns: 
* queryID*100 + center, searchID (s), search start (ss), search end (se)
******************************************************************************/

#include <fstream>
#include <iostream>
#include <unistd.h> // getopt
#include <vector>


int main(int argc, char** argv)
{
    //-------------------------------------------------------------------------
    // Argument parser
    //-------------------------------------------------------------------------

    int opt;
    std::string inFilename;

    while ((opt = getopt(argc, argv, "ho:")) != -1) 
    {
        switch (opt) 
        {
        // case 'o':
        //     outFilename = optarg;
        //     break;
        case 'h':
            // go to default
            std::cout << "Reads dist_Icl binary output file and outputs it to terminal." << std::endl;
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
    uint64_t length = 0;
    char * buffer = NULL;
    uint32_t * pInteger = NULL;
    double * pDouble = NULL;

    // read file
    if (infile) 
    {
        // get length of file:
        infile.seekg (0, infile.end);
        length = infile.tellg();
        infile.seekg (0, infile.beg);

        buffer = new char [length];
        
        std::cerr << "Reading " << length << " characters... ";
        // read data as a block:
        infile.read (buffer,length);
        std::cerr << "all characters read successfully. \n";
        
        pInteger = (unsigned int*) buffer;
        pDouble = (double *) buffer;
    }
    else
    {
      std::cout << "error: only " << infile.gcount() << " could be read \n";
    }
    infile.close();
    
    // each line has 3 unsigned int entries
    uint32_t lines = length/(2*sizeof(uint32_t) + sizeof(double));

    for (uint64_t i = 0; i < lines; ++i)
    {
        // pointer arithmetic depends on the pointer type: 3 unsigned int per line || 6 unsigned short per line.
        std::cout << pInteger[4*i] << ' ' << pInteger[4*i+1] << ' ' << pDouble[2*i + 1] <<'\n';    
    }
    

    delete[] buffer;

  return 0;
}

