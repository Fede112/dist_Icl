/*******************************************************************************
* Reads dist_Icl binary input and outputs it to terminal
* 
* The output consists of 3 columns: 
* queryID*100 + center, searchID (s), search start (ss), search end (se)
******************************************************************************/

#include <fstream>
#include <iostream>
#include <unistd.h> // getopt
#include <vector>

#include <datatypes.h>

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
            std::cout << "Reads dist_Icl binary input file and outputs it to terminal." << std::endl;
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
        std::cout << "Input: " << inFilename << std::endl;
    }

    //-------------------------------------------------------------------------

    
    std::ifstream infile (inFilename, std::ifstream::binary);
    unsigned long int length = 0;
    char * buffer = NULL;
    SmallCA * sca = NULL;

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
        printSCA(sca[i]);
    }
    

    delete[] buffer;

  return 0;
}

