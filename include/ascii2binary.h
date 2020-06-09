#ifndef ASCII2BINARY
#define ASCII2BINARY

#include <string>
#include "smallca.h"

// loads txt to fit into SmallCA structure
void load_txt(std::string fileName, SmallCA * clusterAlign, uint64_t & length);

// calculates number of rows in ascii file
uint64_t rows_txt(std::string fileName);

#endif