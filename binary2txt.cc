#include <fstream>
#include <vector>
#include <iostream>




int main()
{
    const std::size_t kB = 1024;
    const std::size_t MB = 1024 * kB;
    const std::size_t GB = 1024 * MB;

    // for (std::size_t size = 1 * MB; size <= 2 * GB; size *= 2) std::cout << "option1, " << size / MB << "MB: " << option_1(size) << "ms" << std::endl;
    // for (std::size_t size = 1 * MB; size <= 2 * GB; size *= 2) std::cout << "option2, " << size / MB << "MB: " << option_2(size) << "ms" << std::endl;
    // for (std::size_t size = 1 * MB; size <= 2 * GB; size *= 2) std::cout << "option3, " << size / MB << "MB: " << option_3(size) << "ms" << std::endl;


  std::ifstream infile ("out.bin", std::ifstream::binary);
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

    for (int i = 0; i < length/4/4; ++i)
    {
        std::cout << p[i*4] << ' ' << p[i*4+1] << ' ' << p[i*4+2] << ' ' << p[i*4+3] <<'\n';    
    }
    

    delete[] buffer;
  }

  return 0;
}

