#include <fstream>
#include <chrono>
#include <vector>
#include <cstdint>
#include <numeric>
#include <random>
#include <algorithm>
#include <iostream>
#include <cassert>

std::vector<uint64_t> GenerateData(std::size_t bytes)
{
    assert(bytes % sizeof(uint64_t) == 0);
    std::vector<uint64_t> data(bytes / sizeof(uint64_t));
    std::iota(data.begin(), data.end(), 0);
    std::shuffle(data.begin(), data.end(), std::mt19937{ std::random_device{}() });
    return data;
}

long long option_1(std::size_t bytes)
{
    std::vector<uint64_t> data = GenerateData(bytes);

    auto startTime = std::chrono::high_resolution_clock::now();
    auto myfile = std::fstream("file.binary", std::ios::out | std::ios::binary);
    myfile.write((char*)&data[0], bytes);
    myfile.close();
    auto endTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
}

long long option_2(std::size_t bytes)
{
    std::vector<uint64_t> data = GenerateData(bytes);

    auto startTime = std::chrono::high_resolution_clock::now();
    FILE* file = fopen("file.binary", "wb");
    fwrite(&data[0], 1, bytes, file);
    fclose(file);
    auto endTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
}

long long option_3(std::size_t bytes)
{
    std::vector<uint64_t> data = GenerateData(bytes);

    std::ios_base::sync_with_stdio(false);
    auto startTime = std::chrono::high_resolution_clock::now();
    auto myfile = std::fstream("file.binary", std::ios::out | std::ios::binary);
    myfile.write((char*)&data[0], bytes);
    myfile.close();
    auto endTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
}

int main()
{
    const std::size_t kB = 1024;
    const std::size_t MB = 1024 * kB;
    const std::size_t GB = 1024 * MB;

    // for (std::size_t size = 1 * MB; size <= 2 * GB; size *= 2) std::cout << "option1, " << size / MB << "MB: " << option_1(size) << "ms" << std::endl;
    // for (std::size_t size = 1 * MB; size <= 2 * GB; size *= 2) std::cout << "option2, " << size / MB << "MB: " << option_2(size) << "ms" << std::endl;
    // for (std::size_t size = 1 * MB; size <= 2 * GB; size *= 2) std::cout << "option3, " << size / MB << "MB: " << option_3(size) << "ms" << std::endl;


  std::ifstream infile ("outfile.b", std::ifstream::binary);
  if (infile) {
    // get length of file:
    infile.seekg (0, infile.end);
    unsigned int length = infile.tellg();
    infile.seekg (0, infile.beg);

    char * buffer = new char [length];
    unsigned int * p;
    std::cerr << "Reading " << length << " characters... ";
    // read data as a block:
    infile.read (buffer,length);

    if (infile)
      std::cerr << "all characters read successfully. \n";
    else
      std::cout << "error: only " << infile.gcount() << " could be read";
    infile.close();

    p = (unsigned int*)buffer;

    for (int i = 0; i < length/3/4; ++i)
    {
        std::cout << p[i*3] << ' ' << p[i*3+1] << ' ' << p[i*3+2] << '\n';    
    }
    

    delete[] buffer;
  }

  return 0;
}

