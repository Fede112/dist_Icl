#ifndef NORMALIZATION
#define NORMALIZATION

#include "datatypes.h"


// LSB radix sort of record with respect to specified key
void radix_sort(unsigned char * pData, uint64_t count, uint32_t record_size, 
                            uint32_t key_size, uint32_t key_offset = 0);


// compute cluster size and include it in clusterAlign.qSize
void compute_cluster_size(SmallCA * clusterAlign, const uint64_t length);


#endif

