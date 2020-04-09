#include <iostream>
#include <fstream>
#include <unistd.h> // getopt

#include "smallca.h"


int main(int argc, char** argv)
{


    ////////////////////////////////////////////////////////////////////////
    int opt;
    std::string output{"ouput.bin"}; 
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
    ////////////////////////////////////////////////////////////////////////


    SmallCA* clusterAlign;
    uint64_t bufferLen; 

    // define buffer
    bufferLen = rows_txt(input); // counts every '\n'
    std::cout << bufferLen << std::endl;
    clusterAlign = new SmallCA [bufferLen];
    
    // load txt into SmallCA[] buffer: 
    //  qID*100 + centerqSize included
    load_txt(input, clusterAlign, bufferLen);
    for (int i = 0; i < 10; ++i){printSCA(clusterAlign[i]);}

    // calculate qSize values
    compute_cluster_size(clusterAlign, bufferLen);

    for (int i = 0; i < 1000; ++i){printSCA(clusterAlign[i]);}
    
    // std::ofstream out("BIG2_10e5_sorted.bin", std::ios::binary);
    // out.write((char*)&clusterAlign, bufferLen);
    // out.close();

    delete[] clusterAlign;
    return 0;
}
