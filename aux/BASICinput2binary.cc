#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>


using namespace std;


int main(int argc, char** argv)
{


    string input, output;
    
    input=argv[1];
    output=argv[2];
    

    std::ifstream in(input);
    std::ofstream out(output, std::ios::binary);

    unsigned int q,s;
    unsigned short * buff = new unsigned short[3];
    string line;
    unsigned int num_lines=0;

    while (std::getline(in, line)) {
        ++num_lines;
        istringstream iss(line); // make fileline into stream
        //read from stream
        iss >> q >> buff[2] >> s >> buff[0] >> buff[1];
        q = q*100 + buff[2];

        out.write((char*)&q, sizeof(unsigned int));
        out.write((char*)&s, sizeof(unsigned int));
        out.write((char*)buff, 2 * sizeof(unsigned short));

    }

    // if (in.bad()) {
    //     // IO error
    // } else if (!in.eof()) {
    //     // format error (not possible with getline but possible with operator>>)
    // } else {
    //     // format error (not possible with getline but possible with operator>>)
    //     // or end of file (can't make the difference)
    // }

    cout << "Number of lines: " << num_lines << endl;
    in.close();
    out.close();
    return 0;
}
