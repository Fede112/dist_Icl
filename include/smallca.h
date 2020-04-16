#ifndef SMALLCA
#define SMALLCA

#include <iostream>


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


// print a ClusteredAlignment TO STDOUT
inline void printSCA( const SmallCA & clusterAlign) 
{
    std::cout << clusterAlign.qID << " " << clusterAlign.qSize << " " << clusterAlign.sID << " " << clusterAlign.sstart << " " << clusterAlign.send << std::endl;   
}


#endif //SMALLCA


