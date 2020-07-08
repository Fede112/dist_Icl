/* -----------------------------------------------------------------------------
* Temporary script to patch the differences between the output of the primary 
* clustering algorithm and the input of the distance matrix (dist_Icl) algorithm
* for metaclustering.
*
* text input: queryID, center, searchID, searchStart, searchEnd
* binary output: smallCA structure
----------------------------------------------------------------------------- */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream> // istringstream
#include <unistd.h> // getopt

#include "smallca.h"
#include "normalization.h"

//---------------------------------------------------------------------------------------------------------
// ASCII TO BINARY - TEMPORARY FUNCTIONS TO FIX PIPELINE
//---------------------------------------------------------------------------------------------------------

uint64_t rows_txt(std::string fileName)
{
    std::ifstream inFile(fileName);
    if (!inFile)
        throw std::system_error(errno, std::system_category(), "failed to open "+ fileName);

    uint64_t bufferLen =  std::count(std::istreambuf_iterator<char>(inFile), 
        std::istreambuf_iterator<char>(), '\n');

    inFile.close();
    
    // ++ because last \n could be missing
    return ++bufferLen;; 

}

void load_txt(std::string fileName, SmallCA * clusterAlign, uint64_t & length)
{
    uint32_t query,search;
    uint16_t center,sstart,send;

    std::string line;
    uint64_t numLine{0};

    std::ifstream inFile(fileName);
    if (!inFile)
        throw std::system_error(errno, std::system_category(), "failed to open "+ fileName);
   
    std::ios::sync_with_stdio(false);
    while (std::getline(inFile, line)) 
    {
        std::istringstream iss(line); // make fileline into stream
        
        iss >> query >> center >> search >> sstart >> send;
        clusterAlign[ numLine ].qID = query*100 + center;
        clusterAlign[ numLine ].qSize = 0; // compute it later
        clusterAlign[ numLine ].sID = search;
        clusterAlign[ numLine ].sstart = sstart;
        clusterAlign[ numLine ].send = send;
        ++numLine;
    }


    // bufferLen and numLine can differ in one
    length = numLine;
    inFile.close();
    return;   
}




int main(int argc, char** argv)
{

    //-------------------------------------------------------------------------
    // Argument parser
    //-------------------------------------------------------------------------
    int opt;
    std::string output{"output.bin"}; 
    std::string input;

    while ((opt = getopt(argc, argv, "ho:")) != -1) 
    {
        switch (opt) 
        {
        case 'o':
            output = optarg;
            break;
        case 'h':
            // go to default

        default: /* '?' */
            fprintf(stderr, "Usage: %s input.txt -o output.bin \n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) 
    {
        fprintf(stderr, "Expected argument after options\n");
        exit(EXIT_FAILURE);
    }

    printf("name argument = %s\n", argv[optind]);
    input = argv[optind];
    
    //-------------------------------------------------------------------------
    
    SmallCA* clusterAlign;
    uint64_t bufferLen; 

    // define buffer
    bufferLen = rows_txt(input); // counts every '\n'
    std::cout << bufferLen << std::endl;
    clusterAlign = new SmallCA [bufferLen];
    
    // load txt into SmallCA[] buffer: qID*100 + center ; qSize included  
    load_txt(input, clusterAlign, bufferLen);
    
    // sort wrt qID+center
    if (    !std::is_sorted( clusterAlign, clusterAlign+bufferLen, compare_qID() )    )
    {   
        radix_sort((unsigned char*) clusterAlign, bufferLen, 16, 4, 0);
    }
    
    // compute size of each primary cluster: qSize
    compute_cluster_size(clusterAlign, bufferLen);

    // sort back wrt sID
    radix_sort((unsigned char*) clusterAlign, bufferLen, 16, 4, 8);
    

    std::ofstream out(output, std::ios::binary);
    std::cout << "sizeof SmallCA: " << sizeof(SmallCA) << std::endl;
    out.write((char*)clusterAlign, bufferLen*sizeof(SmallCA));
    out.close();


    delete[] clusterAlign;
    return 0;
}
