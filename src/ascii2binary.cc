#include <sstream> // istringstream
#include <fstream>
#include <algorithm>

#include "ascii2binary.h"

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








