#ifndef DTYPES
#define DTYPES

#include <iostream>

struct MatchedPair
{
    uint32_t ID1;
    uint32_t ID2;
    uint32_t normFactor;

    MatchedPair(): ID1{0}, ID2{0}, normFactor{0}{};
    MatchedPair(uint32_t id1, uint32_t id2, uint32_t n): ID1(id1), ID2(id2), normFactor(n) {}
};

struct NormalizedPair
{
    uint32_t ID1;
    uint32_t ID2;
    double distance;

    NormalizedPair()=default;
    NormalizedPair(uint32_t id1, uint32_t id2, double d):  ID1(id1), ID2(id2), distance(d) {}
};


struct Ratio
{
    uint32_t num;
    uint32_t denom;
    
    Ratio()=default;
    Ratio(uint32_t n, uint32_t d):  num(n), denom(d) {}

    inline double as_double() const {return (double)num/denom;}
};

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



struct compare_sID // used by auxiliary kmerge_binary
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


// print a ClusteredAlignment TO STDOUT
inline void printSCA( const SmallCA & clusterAlign) 
{
    std::cout << clusterAlign.qID << " " << clusterAlign.qSize << " " << clusterAlign.sID << " " << clusterAlign.sstart << " " << clusterAlign.send << std::endl;   
}


#endif //DTYPES


