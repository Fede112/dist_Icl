#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "cxxopts.hpp"
using namespace std;


int main(int argc, char** argv)
{


    ////////////////////////////////////////////////////////////////////////
    // Args parser (I tried a new library but I am not convinced)

    cxxopts::Options options("txt2binary", 
        "Converts dist_Icl txt input file into binary format.\n\n"
        "Input file has 5 columns: q, c, s, ss, se \n"
        "Output file has 4 columns q*100+c, s, ss, se.\n");

    std::string input, output;
    options.add_options()
        ("h,help", "Print usage")
        ("i,input", "Input file path", cxxopts::value<std::string>())
        ("o,output", "Output file path", cxxopts::value<std::string>())
    ;

    
    auto args = options.parse(argc, argv);


    if (args.count("help"))
    {
      std::cout << options.help() << std::endl;
      exit(0);
    }
    if (args.count("input")==1)
        input = args["input"].as<std::string>();
    else
    {
        std::cout << "Missing input flag: -i <filepath>" << std::endl;
        exit(1);
    }
    if (args.count("output")==1)
        output = args["output"].as<std::string>();
    else
    {
        std::cout << "Missing input flag: -o <filepath>" << std::endl;
        exit(1);
    }
    ////////////////////////////////////////////////////////////////////////


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
