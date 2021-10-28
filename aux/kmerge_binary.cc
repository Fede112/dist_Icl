#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <list>
#include <random>
#include <functional>
#include <math.h>    
#include <unistd.h> // getopt
 
#include <datatypes.h>


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

        for (uint64_t i = 1; i < offsetBase2.size(); ++i)
        {
            std::vector<Iterator> localPartIndexes(partIndexes.begin() + offsetBase2[i-1], 
                                                                        partIndexes.begin() + offsetBase2[i] + 1);

            kmerge_pow2(localPartIndexes, cmp);    
        }
        
        kmerge(mergedPartIndexes, cmp);
    }
    
}


int main(int argc, char *argv[])
{

    //-------------------------------------------------------------------------
    // Argument parser

    int opt;
    std::string outFilename={"merged_output.bin"};
    std::vector<std::string> filenames;

    while ((opt = getopt(argc, argv, "ho:")) != -1) 
    {
        switch (opt) 
        {
        case 'o':
            outFilename = optarg;
            break;
        case 'h':
            // go to default

        default: /* '?' */
            std::cerr << "Usage: \n";
            std::cerr << "\t " << argv[0] << " INPUT1 INPUT2 ... [-o OUTPUT] \n\n";
            std::cerr << "\t INPUTs     list of files to be merged \n\n";
            std::cerr << "\t -o OUTPUT  output filename (it will use a default name otherwise) \n\n";
            std::cerr << "Description:\n\t k-way merge of dist_Icl output binary files.\n\n";
            std::cerr << "\t Performance can be improved considerably if compiled with OpenMP.\n";
            std::cerr << "\t Important: files need to be sorted wrt searchID in order for the merge to succeed!\n\n";
            exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected arguments after options\n");
        exit(EXIT_FAILURE);
    }

    while(optind < argc)
    {
        printf("name argument = %s\n", argv[optind]);
        filenames.push_back(argv[optind]);
        ++optind;
    }

    //-------------------------------------------------------------------------
    
    std::vector<SmallCA*> indexes; 
    std::vector<std::ifstream> files; 
    std::vector<uint64_t> fileLines; 
    uint64_t bytes{0};
    uint64_t lines{0};
    uint64_t totalLines{0};
    for (const auto & filename: filenames)
    {
        std::cout << filename << std::endl;
        std::ifstream file (filename, std::ifstream::binary);
        if(file.is_open())
        {
            file.seekg (0, file.end);
            bytes = file.tellg();
            lines = bytes/sizeof(SmallCA);
            // std::cout << lines << std::endl;
            fileLines.push_back(lines);
            totalLines += lines;
            file.seekg (0, file.beg);
            files.push_back(std::move(file));
        }
        else{std::cerr << "Cannot open file: " << filename << "\n";}
    }
    

    SmallCA * buffer = new SmallCA [totalLines];
    
    indexes.push_back(buffer);
    auto subBuffer = buffer;
    for (uint64_t i = 0; i < files.size(); ++i)
    {
        files[i].read ((char*) subBuffer, sizeof(SmallCA)*fileLines[i]);
        subBuffer += fileLines[i];
        indexes.push_back(subBuffer);
    }

    kmerge(indexes, compare_sID());

    std::cout << "Check if file is sorted: " << std::is_sorted(buffer, buffer+totalLines, compare_sID()) << std::endl;

    std::ofstream out(outFilename, std::ios::binary);
    out.write((char*)buffer, totalLines*sizeof(SmallCA));
    out.close();


    return 0;

}