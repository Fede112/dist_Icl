/*******************************************************************************
* Auxiliary function needed to make original input compatible with binary input.
* 
* Converts original dist_Icl input text file into binary format.
*
* QueryID and center are combined into one column. The output consists of 3 columns: 
* queryID*100 + center, searchID (s), search start (ss), search end (se)
******************************************************************************/

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <unistd.h> // getopt

int main(int argc, char** argv)
{

    //-------------------------------------------------------------------------
    // Argument parser
    //-------------------------------------------------------------------------

    int opt;
    std::string outFilename = {"output.bin"};
    std::string inFilename;

    while ((opt = getopt(argc, argv, "ho:")) != -1) 
    {
        switch (opt) 
        {
        case 'o':
            outFilename = optarg;
            break;
        case 'h':
            std::cout << "Converts dist_Icl input text file into binary format." << std::endl;
            std::cout << "QueryID and center are combined into one column: qID*100+c." << std::endl;
            std::cout << "Usage: " << argv[0] << " file.txt [-o output binary file]" << std::endl;
            break;

        default: /* '?' */
            std::cout << "Usage: " << argv[0] << " file.txt [-o output binary file]" << std::endl;
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


    std::ifstream in(inFilename);
    std::ofstream out(outFilename, std::ios::binary);

    uint32_t q,s;
    uint16_t * buff = new uint16_t[3];
    std::string line;
    uint32_t num_lines=0;

    // read file
    while (std::getline(in, line)) 
    {
        ++num_lines;
        std::istringstream iss(line); // make fileline into stream
        //read from stream
        iss >> q >> buff[2] >> s >> buff[0] >> buff[1];
        q = q*100 + buff[2];

        out.write((char*)&q, sizeof(uint32_t));
        out.write((char*)&s, sizeof(uint32_t));
        out.write((char*)buff, 2 * sizeof(uint16_t));

    }

    // if (in.bad()) {
    //     // IO error
    // } else if (!in.eof()) {
    //     // format error (not possible with getline but possible with operator>>)
    // } else {
    //     // format error (not possible with getline but possible with operator>>)
    //     // or end of file (can't make the difference)
    // }

    std::cout << "Number of lines: " << num_lines << std::endl;
    in.close();
    out.close();
    return 0;
}
