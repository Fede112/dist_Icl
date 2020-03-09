#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;


int main()
{
   std::ifstream in("./data/BIG1_map2.clean");
   std::ofstream out("out.bin", std::ios::binary);



	// int is 4 bytes. long int is already 8 bytes
	unsigned int *p = new unsigned int[4]; //*lines];

    string line;
    while(!in.eof())
    {
    	getline(in, line);
	    istringstream iss(line); // make fileline into stream
	    //read from stream
	    iss >> p[0] >> p[1] >> p[2] >> p[3];
	    out.write((char*)p, 4 * sizeof(unsigned int));
	}

	in.close();
	out.close();
    return 0;
}
