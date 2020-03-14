#ifndef SMALLCA
#define SMALLCA

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
void printSCA( const SmallCA & SCA) {

    std::cout << SCA.qID << " " << SCA.sID << " " << SCA.sstart << " " << SCA.send << std::endl;
    
}

#endif //SMALLCA