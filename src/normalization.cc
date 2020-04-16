#include <iostream>
#include <cstring> // memcpy
#include <algorithm> // bounds
// #include <cstddef>
// #include <cstdint>

#include "normalization.h"

//---------------------------------------------------------------------------------------------------------
// NORMALIZATION AUXILIARY FUNCTIONS
//---------------------------------------------------------------------------------------------------------

void radix_sort(unsigned char * pData, uint64_t count, uint32_t record_size, 
                            uint32_t key_size, uint32_t key_offset)
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

//---------------------------------------------------------------------------------------------------------
// TODO: FULL C++ INTERFACE
//---------------------------------------------------------------------------------------------------------

// // forcing move semantics (if compiler is old this is not guaranteed) 
// // TODO: unsigned char * pData -> vector<SmallCA>
// void radix_sort(std::vector<unsigned int> & data)
// {

//     std::size_t m,n;
//     unsigned int key;
//     std::size_t keySize{sizeof(key)};
//     std::size_t count{data.size()};


//     // we want the sorted keys in an std::container to match elementFrequency() interface
//     // std::vector <unsigned int> sortedKeys(count);
//     std::vector <unsigned int> temp(count);
//     // sortedKeys.reserve(count);
//     // temp.reserve(count);

//     // index matrix
//     size_t mIndex[keySize][256] = {0};            
    

//     // generate histograms
//     for(std::size_t i = 0; i < count; i++)                  
//     {         
//         key = data[i];
//         for(std::size_t j = 0; j < keySize; j++)
//         {
//             mIndex[j][(size_t)(key & 0xff)]++;
//             key >>= 8;
//         }       
//     }
//     // convert to indices 
//     for(std::size_t j = 0; j < keySize; j++)             
//     {
//         n = 0;
//         for(std::size_t i = 0; i < 256; i++)
//         {
//             m = mIndex[j][i];
//             mIndex[j][i] = n;
//             n += m;
//         }       
//     }

//     // Radix sort
//     for(std::size_t j = 0; j < keySize; j++)    
//     {
//         for(std::size_t i = 0; i < count; i++)
//         {
//             key = data[i];
//             m = (size_t)(key >> (j<<3)) & 0xff;
//             temp[mIndex[j][m]++] = key;
//         }
//         std::swap(data, temp);
//     }
    
//     return;
// }


// // Function to find frequency of each element in a sorted array (cpp)
// std::unordered_map<unsigned int, unsigned int> 
//     find_frequency(std::vector<unsigned int> const &sortedVector)
// {
//     // map to store frequency of each element of the array
//     std::unordered_map<unsigned int, unsigned int> count;

//     // do for every distinct element in the array
//     auto it = sortedVector.begin();
//     while (it != sortedVector.end())
//     {
//         unsigned int val = *it;

//         // find first occurrence
//         auto low = std::lower_bound(sortedVector.begin(), sortedVector.end(), val);

//         // find last occurrence
//         auto high = std::upper_bound(sortedVector.begin(), sortedVector.end(), val);

//         // update the difference in the map
//         count[val] = high - low;

//         // important - move to next element in vector (not immediate next)
//         it = it + count[val];
//     }

//     // return the map
//     return count;
// }



// bool check_order(std::vector<unsigned int> data)
// {
//     for (std::size_t i = 1; i < data.size(); ++i)
//     {
//         if(data[i] < data[i-1])
//             return false;
//     }    
//     return true;
// }




// int main(int argc, char** argv) 
// {
//     // Load input file
//     std::ifstream infile (argv[1], std::ifstream::binary);
//     unsigned long int bytes{0};
//     if (infile)
//     {
//         infile.seekg (0, infile.end);
//         bytes = infile.tellg();
//         infile.seekg (0, infile.beg);
//         bufferc c = new char [bytes];
//         linesA = bytes/sizeof(SmallCA);

//         std::cerr << "Reading " << bytes << " characters... ";
//         // read data as a block:
//         infile.read (bufferc c,bytes);
//         std::cerr << "all characters read successfully from " << argv[1] << "\n";
//     }
//     else
//     {
//       std::cout << "error: only " << infile.gcount() << " could be read";
//       return 1;
//     }
//     infile.close();

//     return 0;
// }