#include <iostream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <list>
#include <random>
#include <functional>
#include <math.h>       /* log2 */
#include <unistd.h> // getopt
 

template <typename Iterator, typename Comparator = std::less<>>
void kmerge_pow2(std::vector<Iterator> & indexes, Comparator cmp = Comparator())
{
    
    int partitions = indexes.size() - 1;
    int merges = log2(partitions);

    // merging iterations
    for (int j = 0; j < merges; ++j)
    {
        // parallel merge
        #pragma omp parallel for
        for (int i = 0; i < partitions/2; ++i)
        {
            std::inplace_merge(indexes[2*i], indexes[2*i+1], indexes[2*i+2], cmp);
        }

        // after merge, erase the middle point indexes
        for (int i = 0; i < partitions/2; ++i)
        {
            // for small number of files , k~100, it is not a bottleneck to do std::erase.
            indexes.erase(indexes.begin() + 2*i+1 - i);
        }

        // update the number of partitions
        partitions = partitions/2;
    }

    return;
}

// template <typename Iterator, typename Compare = std::less<typename std::iterator_traits<Iterator>::value_type> >
template <typename Iterator, typename Comparator = std::less<>>
void kmerge(std::vector<Iterator> & partIndexes, Comparator cmp = Comparator() )
{

    int partitions = partIndexes.size() - 1;
    if (partitions == 1)
        return;
    else
    {
        std::vector<Iterator> mergedPartIndexes{partIndexes[0]};
        std::vector<int> offsetBase2{0};
        int accumMask = 0;
        for (int i = 0; i < 32; i++)
        {
            int mask = 1 << i;
            if ((partitions & mask) != 0)
            {
                accumMask += mask;
                mergedPartIndexes.push_back(partIndexes[0  + accumMask ]);
                offsetBase2.push_back(accumMask);
            }
        }

        for (int i = 1; i < offsetBase2.size(); ++i)
        {
            std::vector<Iterator> localPartIndexes(partIndexes.begin() + offsetBase2[i-1], 
                                                                        partIndexes.begin() + offsetBase2[i] + 1);

            kmerge_pow2(localPartIndexes, cmp);    
        }
        
        kmerge(mergedPartIndexes);
    }
    
}


int main(int argc, char *argv[])
{
    int opt;
    std::string outFileName;
    std::vector<std::string> fileNames;

    while ((opt = getopt(argc, argv, "ho:")) != -1) 
    {
        switch (opt) 
        {
        case 'o':
            outFileName = optarg;
            break;
        case 'h':
            // go to default

        default: /* '?' */
            fprintf(stderr, "Usage: %s file1 file2 ... [-o output merged file] \n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(EXIT_FAILURE);
    }

    while(optind < argc)
    {
        printf("name argument = %s\n", argv[optind]);
        fileNames.push_back(argv[optind]);
        ++optind;
    }

    
    // vector<ifstream> files; 
    // uint64_t totalBytes{0};
    // for (const auto & fileName: fileNames)
    // {
    //     std::ifstream file (fileName, std::ifstream::binary);
    //     if(file.is_open())
    //     {
    //         file.seekg (0, file.end);
    //         totalBytes += file.tellg();
    //         file.seekg (0, file.beg);
    //         files.push_back(file); 
    //     }
    //     else{std::cerr << "Cannot open file: " << fileName << "\n";}
    // }
    
    // char * buffer = new char [totalBytes];
    
    


    // fill the vectors with random numbers
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<> dis(0, 2900);
 
    std::vector<int> v1(2900);
    std::vector<std::vector<int>::iterator> indexes;
    std::generate(v1.begin(), v1.end(), std::bind(dis, std::ref(mt)));

    int k = 29;
    for (int i = 0; i <= k; ++i)
    {
        indexes.push_back(v1.begin()+i*100);
    }
    
    for (int i = 0; i < k; ++i)
    {
     std::sort(indexes[i], indexes[i+1]);
    }

    // for (const auto & elem: v1)
    // {
    //     std::cout << elem << std::endl;
    // }


    kmerge(indexes);

    
    for (const auto & elem: v1)
    {
        std::cout << elem << std::endl;
    }

    std::cout << std::is_sorted(v1.begin(), v1.end()) << std::endl;



    return 0;



}