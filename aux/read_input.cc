// -----------------------------------------------------------------------------
// Reads dist_Icl binary input and outputs it to terminal
// 
// The output consists of 4 columns: 
// queryID*100 + center, searchID (s), search start (ss), search end (se)
// -----------------------------------------------------------------------------

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
            // go to default

        default: /* '?' */
            std::cout << "\nUsage: " << argv[0] << " INPUT \n\n";
            std::cerr << "\t INPUT      input filename \n\n";
            std::cerr << "Description:\n\t" << argv[0] << " reads dist_Icl binary input and outputs it to terminal.\n\n";
            std::cerr << "\t input format: queryID+center, query size, searchID, searchStart, searchEnd.\n";
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
      std::cout << "error: only " << infile.gcount() << " could be read\n";
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

