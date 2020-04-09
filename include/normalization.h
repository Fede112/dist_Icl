#ifndef NORMALIZATION
#define NORMALIZATION


#include <vector>
#include <cstddef>

void radix_sort(std::vector<unsigned int> & data);

std::unordered_map<unsigned int, unsigned int> 
	find_frequency(std::vector<unsigned int> const &sortedVector);

bool check_order(std::vector<unsigned int> data);

#endif

