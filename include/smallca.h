#ifndef SMALLCA
#define SMALLCA

#include <iostream>
#include <algorithm>


// For .cc
#include <sstream> // istringstream
#include <fstream>
#include <cstring> // memcpy

// #include <sstream>


// Small Clustered Alignment structure (only essential data...)
struct SmallCA
{
    uint32_t qID; // qID*100 + center
    uint32_t qSize;
    uint32_t sID;
    uint16_t sstart;
    uint16_t send;
    
    SmallCA(uint32_t q, uint32_t qs, uint32_t s, uint16_t ss, uint16_t se): qID(q), qSize(qs), sID(s), sstart(ss), send(se){}
    SmallCA(): qID(0), qSize(0),sID(0), sstart(0), send (0) {}

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

// needed for std::upper_bound in compute_cluster_size
struct compare_sID
{
    bool operator() (const SmallCA & left, const SmallCA & right)
    {
        return left.sID < right.sID;
    }
    bool operator() (const SmallCA & left, uint32_t right)
    {
        return left.sID < right;
    }
    bool operator() (uint32_t left, const SmallCA & right)
    {   
        return left < right.sID;
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

        // compute the difference
        auto count = high - low;

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

struct ClusterPairs
{
    uint64_t ID;
    uint32_t norm;

    ClusterPairs(): ID(0), norm(0) {}
};

struct MatchedPair
{
    uint32_t ID1;
    uint32_t ID2;
    double distance;

    MatchedPair(): ID1(0), ID2(0), distance(0) {}
    MatchedPair(uint32_t id1, uint32_t id2, double d): ID1(id1), ID2(id2), distance(d) {}
};


struct compare_pair
{
    bool operator() (const ClusterPairs & left, const ClusterPairs & right)
    {
        return left.ID < right.ID;
    }
    bool operator() (const ClusterPairs & left, uint64_t right)
    {
        return left.ID < right;
    }
    bool operator() (uint64_t left, const ClusterPairs & right)
    {   
        return left < right.ID;
    }
};

// void frequency(ClusterPairs * buffer, const uint64_t length, std::vector<NormalizedPairs> & unique )
// {
//     auto end = buffer + length;
//     auto low = buffer;

//     while (low != end)
//     {
//         uint64_t val = low->ID;
//         uint32_t norm = low->norm;

//         // find last occurrence
//         auto high = std::upper_bound(low, end, val, compare_pair());

//         // compute the difference
//         auto count = high - low;

//         unique.push_back(NormalizedPairs(val, (double)count/norm));
        
//         // move to next element in vector (not immediate next)
//         low = low + count;
//     }

//     return;
// }



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


void radix_sort(unsigned char * pData, uint64_t count, uint32_t record_size, 
                            uint32_t key_size, uint32_t key_offset = 0)
{
    typedef uint8_t k_t [key_size];
    typedef uint8_t record_t [record_size];

    // index matrix
    uint64_t mIndex[key_size][256] = {0};            
    unsigned char * pTemp = new unsigned char [count*sizeof(record_t)];
    unsigned char * pDst, * pSrc;
    uint64_t i,j,m,n;
    k_t u;
    record_t v;

    // generate histograms
    for(i = 0; i < count; i++)
    {         
        std::memcpy(&u, (pData + record_size*i + key_offset), sizeof(u));
        
        for(j = 0; j < key_size; j++)
        {
            mIndex[j][(size_t)(u[j])]++;
            // mIndex[j][(size_t)(u & 0xff)]++;
            // u >>= 8;
        }       
    }
    // convert to indices 
    for(j = 0; j < key_size; j++)
    {
        n = 0;
        for(i = 0; i < 256; i++)
        {
            m = mIndex[j][i];
            mIndex[j][i] = n;
            n += m;
        }       
    }

    // radix sort
    pDst = pTemp;                       
    pSrc = pData;
    for(j = 0; j < key_size; j++)
    {
        for(i = 0; i < count; i++)
        {
            std::memcpy(&u, (pSrc + record_size*i + key_offset), sizeof(u));
            std::memcpy(&v, (pSrc + record_size*i), sizeof(v));
            m = (size_t)(u[j]);
            // m = (size_t)(u >> (j<<3)) & 0xff;
            std::memcpy(pDst + record_size*mIndex[j][m]++, &v, sizeof(v));
        }
        std::swap(pSrc, pDst);
        
    }
    delete[] pTemp;
    return;
}


#endif //SMALLCA


