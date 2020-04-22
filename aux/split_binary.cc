#include "smallca.h"
#include <fstream>
#include <libgen.h>
#include <assert.h>     /* assert */

// // input (contain clustered alignments); say "B" files (B as Block, eaach block contains data foro
//     std::ifstream infileA (argv[1], std::ifstream::binary);
//     std::ifstream infileB (argv[2], std::ifstream::binary);
//     unsigned long int bytesA{0}, bytesB{0};
//     if (infileA)
//     {
//         infileA.seekg (0, infileA.end);
//         bytesA = infileA.tellg();
//         infileA.seekg (0, infileA.beg);
//         bufferA = new char [bytesA];
//         linesA = bytesA/sizeof(SmallCA);

//         std::cerr << "Reading " << bytesA << " characters... ";
//         // read data as a block:
//         infileA.read (bufferA,bytesA);
//         std::cerr << "all characters read successfully from " << argv[1] << "\n";
//     }
//     else
//     {
//       std::cout << "error: only " << infileA.gcount() << " could be read";
//       return 1;
//     }
//     infileA.close();
    

template<class T>
T base_name(T const & path, T const & delims = "/\\")
{
    return path.substr(path.find_last_of(delims) + 1);
}
template<class T>
T remove_extension(T const & filename)
{
    typename T::size_type const p(filename.find_last_of('.'));
    return p > 0 && p != T::npos ? filename.substr(0, p) : filename;
}

int main(int argc, char const *argv[])
{
    int k = 5;

    std::string inFilename = argv[1];
    std::ifstream inFile (inFilename, std::ifstream::binary);
    uint64_t bytes, lines;
    char * buffer{NULL};
    if (inFile)
    {
        inFile.seekg (0, inFile.end);
        bytes = inFile.tellg();
        inFile.seekg (0, inFile.beg);
        buffer = new char [bytes];
        lines = bytes/sizeof(SmallCA);
        // read data as a block:
        inFile.read (buffer,bytes);   
    }
    else
    {
      std::cout << "error: couldn't read " << inFilename << std::endl;
      return 1;
    }

    inFile.close();

    auto linesPerFile = lines / k;
    auto buffer_tmp = buffer;
    uint64_t accLines {0};
    for (int i = 0; i < k; ++i)
    {
        if (i==k)
            linesPerFile += lines%k;
        auto outFilename = remove_extension(base_name(inFilename)) + "_split_" + std::to_string(i) + ".bin";
        outFilename = "./test_data/" + outFilename;
        std::cout << outFilename << std::endl;
        auto outFile = std::fstream(outFilename, std::ios::out | std::ios::binary);

        outFile.write(buffer_tmp, linesPerFile*sizeof(SmallCA));
        outFile.close();
        buffer_tmp += linesPerFile*sizeof(SmallCA);
        accLines += linesPerFile;
    }

    assert(accLines == lines);

    delete[] buffer;
    return 0;
}