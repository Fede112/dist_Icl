#include <vector>
#include <iostream>
#include <cstddef>
#include <cstring> // memcpy
#include <unordered_map>
#include <algorithm> // bounds
// #include <cstdint>


//---------------------------------------------------------------------------------------------------------
// NORMALIZATION AUXILIARY FUNCTIONS
//---------------------------------------------------------------------------------------------------------

// forcing move semantics (if compiler is old this is not guaranteed) 
// TODO: unsigned char * pData -> vector<SmallCA>
void radix_sort(std::vector<unsigned int> & data)
{

    std::size_t m,n;
    unsigned int key;
    std::size_t keySize{sizeof(key)};
    std::size_t count{data.size()};


    // we want the sorted keys in an std::container to match elementFrequency() interface
    // std::vector <unsigned int> sortedKeys(count);
    std::vector <unsigned int> temp(count);
    // sortedKeys.reserve(count);
    // temp.reserve(count);

    // index matrix
    size_t mIndex[keySize][256] = {0};            
    

    // generate histograms
    for(std::size_t i = 0; i < count; i++)                  
    {         
        key = data[i];
        for(std::size_t j = 0; j < keySize; j++)
        {
            mIndex[j][(size_t)(key & 0xff)]++;
            key >>= 8;
        }       
    }
    // convert to indices 
    for(std::size_t j = 0; j < keySize; j++)             
    {
        n = 0;
        for(std::size_t i = 0; i < 256; i++)
        {
            m = mIndex[j][i];
            mIndex[j][i] = n;
            n += m;
        }       
    }

    // Radix sort
    for(std::size_t j = 0; j < keySize; j++)    
    {
        for(std::size_t i = 0; i < count; i++)
        {
            key = data[i];
            m = (size_t)(key >> (j<<3)) & 0xff;
            temp[mIndex[j][m]++] = key;
        }
        std::swap(data, temp);
    }
    
    return;
}


// Function to find frequency of each element in a sorted array (cpp)
std::unordered_map<unsigned int, unsigned int> 
    find_frequency(std::vector<unsigned int> const &sortedVector)
{
    // map to store frequency of each element of the array
    std::unordered_map<unsigned int, unsigned int> count;

    // do for every distinct element in the array
    auto it = sortedVector.begin();
    while (it != sortedVector.end())
    {
        unsigned int val = *it;

        // find first occurrence
        auto low = std::lower_bound(sortedVector.begin(), sortedVector.end(), val);

        // find last occurrence
        auto high = std::upper_bound(sortedVector.begin(), sortedVector.end(), val);

        // update the difference in the map
        count[val] = high - low;

        // important - move to next element in vector (not immediate next)
        it = it + count[val];
    }

    // return the map
    return count;
}



bool check_order(std::vector<unsigned int> data)
{
    for (std::size_t i = 1; i < data.size(); ++i)
    {
        if(data[i] < data[i-1])
            return false;
    }    
    return true;
}




int main(int argc, char** argv) 
{
    // Load input file
    std::ifstream infile (argv[1], std::ifstream::binary);
    unsigned long int bytes{0};
    if (infile)
    {
        infile.seekg (0, infile.end);
        bytes = infile.tellg();
        infile.seekg (0, infile.beg);
        bufferc c = new char [bytes];
        linesA = bytes/sizeof(SmallCA);

        std::cerr << "Reading " << bytes << " characters... ";
        // read data as a block:
        infile.read (bufferc c,bytes);
        std::cerr << "all characters read successfully from " << argv[1] << "\n";
    }
    else
    {
      std::cout << "error: only " << infile.gcount() << " could be read";
      return 1;
    }
    infile.close();

    return 0;
}