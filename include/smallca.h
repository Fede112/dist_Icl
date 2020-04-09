#ifndef SMALLCA
#define SMALLCA

#include <iostream>
#include <algorithm>


// For .cc
#include <sstream> // istringstream
#include <fstream>
// #include <sstream>


// Small Clustered Alignment structure (only essential data...)
struct SmallCA
{
    unsigned int qID;
    unsigned int qSize;
    unsigned int sID;
    unsigned short sstart;
    unsigned short send;
    
    SmallCA(unsigned int q, unsigned int qs, unsigned int s, unsigned short ss, unsigned short se): qID(q), qSize(qs), sID(s), sstart(ss), send(se){}
    SmallCA(): qID(0), qSize(0),sID(0), sstart(0), send (0) {};

};


// print a ClusteredAlignment TO STDOUT
void printSCA( const SmallCA & clusterAlign) {

    std::cout << clusterAlign.qID << " " << clusterAlign.qSize << " " << clusterAlign.sID << " " << clusterAlign.sstart << " " << clusterAlign.send << std::endl;
    
}



// needed for std::upper_bound in compute_cluster_size
struct compare_qID
{
    bool operator() (const SmallCA & left, const SmallCA & right)
    {
        return left.qID < right.qID;
    }
    bool operator() (const SmallCA & left, uint32_t right)
    {
        return left.qID < right;
    }
    bool operator() (uint32_t left, const SmallCA & right)
    {   
        return left < right.qID;
    }
};


void compute_cluster_size(SmallCA * clusterAlign, const uint64_t length)
{
    auto end = clusterAlign + length;
    auto low = clusterAlign;

    while (low != end)
    {
        uint32_t val = low->qID;

        // find last occurrence
        auto high = std::upper_bound(low, end, val, compare_qID());
        // std::cout << high->qID << std::endl;

        // compute the difference
        auto count = high - low;
        // std::cout << count << std::endl;

        // add cluster size to the list
        for (auto p = low; p <= high; ++p)
        {
        	p->qSize = count;
        }

        // move to next element in vector (not immediate next)
        low = low + count;
    }

    return;
}



uint64_t rows_txt(std::string fileName)
{
    std::ifstream inFile(fileName);
    if (!inFile)
        throw std::system_error(errno, std::system_category(), "failed to open "+ fileName);

    uint64_t bufferLen =  std::count(std::istreambuf_iterator<char>(inFile), 
        std::istreambuf_iterator<char>(), '\n');

    inFile.close();
    
    return ++bufferLen;; // last \n could be missing

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


#endif //SMALLCA


