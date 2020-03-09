#include <fstream>
#include <vector>
#include <iostream>

// Small Clustered Alignment structure (only essential data...)
struct SmallCA
{
    unsigned int qID;
    unsigned int sID;
    unsigned short sstart;
    unsigned short send;
    
    SmallCA(unsigned int q, unsigned int s, unsigned short ss, unsigned short se): qID(q), sID(s), sstart(ss), send (se) {}

};


//print a ClusteredAlignment TO STDOUT
void printSCA( SmallCA SCA) {

    std::cout << SCA.qID << " " << SCA.sID << " " << SCA.sstart << " " << SCA.send << std::endl;
    
}



int main()
{





  std::ifstream infile ("./data/BIG2_10e7.bin", std::ifstream::binary);
  if (infile) {
    // get length of file:
    infile.seekg (0, infile.end);
    unsigned long int length = infile.tellg();
    infile.seekg (0, infile.beg);

    char * buffer = new char [length];
    unsigned int * p = (unsigned int*)buffer;
    unsigned short * pos = (unsigned short*)buffer;
    SmallCA * sca = (SmallCA*)buffer;
    std::cerr << "Reading " << length << " characters... ";
    // read data as a block:
    infile.read (buffer,length);

    if (infile)
      std::cerr << "all characters read successfully. \n";
    else
      std::cout << "error: only " << infile.gcount() << " could be read";
    infile.close();
    unsigned long int bytes_line = 2*sizeof(unsigned int) + 2*sizeof(unsigned short);


    for (int i = 0; i < length/(bytes_line); ++i)
    {
        // pointer arithmetic depends on the pointer type: 3 unsigned int per line || 6 unsigned short per line.
        std::cout << p[3*i] << ' ' << p[3*i + 1] << ' ' << pos[6*i + 4] << ' ' << pos[6*i + 4 + 1] <<'\n';    
        // printSCA(sca[i]);


    }
    

    delete[] buffer;
  }

  return 0;
}

