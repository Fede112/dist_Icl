## Dist_Icl Notes

### Final pipeline

- [x] Start from original 200 files ALREADY sorted by cluster id  (aX.txt where X i from 1 to 200)

- [x] compute cluster size (all this is available in ./pipeline/)
  needs: convert aX.txt to aX.bin - this binary includes the empty qSize column (OK);  `load_txt()`
  compute cluster sizes (TODO) - print binary output ( bX.bin ) which will have 1 extra column `compute_cluster_size()`

- [x] sort back wrt sID `radix_sort()`

- [x] sort bX.bin wrt sID / merge 200 files in in 2 files sorted wrt qID - `./aux/merge_binary.cc`
  (A) using Fede's programs (trying this)

  ~~(B) convert bX.bin bX.txt (need proper program to do it); use  bash  ascii sort ; at the end reconvert everything <-(new conversion  programs with 1 extra column)~~

- [x] compute dist:

  * speed up code



### TODO list

- [x] Change metaclustering output

- [x] Run it on the big data set to test times (results are not important yet)

- [x] Consistency test: P53 comparisson

- [x] Run Elena's code from primary clustering

- [ ] 

- [ ] map results with original input to check the ground truth

- [ ] Merging the metaclusters

- [ ] Clean up git (converge to master)

- [ ] **modification**: remove `#ifdef DIAGONAL` and solve it just by comparing the input strings. You just add an if condition to the not diagonal code.

- [ ] **Major modification**: map of maps is not the most efficient container. For counting it is probably much faster to use unordered_map and afterwards sort using `radix_sort` 

- [ ] **Major modification**: it is probably more efficient to keep qSize in a separate container (unordered_map) and normalize the distance of each pair after the producer threads are done.
  * avoid norm calculation in producer
  * The qSize search is done in parallel by each consumer

- [ ] Add unit tests

- [ ] Full C++ migration. The code handles a combination of raw buffers with pointers arithmetics or std containers with assignment. The idea is to wrap the raw buffers with std containers.

  One key problem was loading a binary file into a vector. Apparently you can do it like this:

```cpp
std::vector<double> buf(N / sizeof(double));// reserve space for N/8 doubles
infile.read(reinterpret_cast<char*>(buf.data()), buf.size()*sizeof(double)); // or &buf[0] for C++98
```

[reference](https://stackoverflow.com/questions/28707928/how-to-efficiently-read-a-binary-file-into-a-vector-c)

​	Functions, like bsort, using c++ iterators interface: `func(iter begin, iter end, compare())`

- [ ] Merge compare_qID and compare_sID into one class

- [ ] wrap concurrent queue into a struct that has queue.index as a member. This will enable the consumer to receive as argument only its queue.  Check if the wrapping impacts the performance.

- [ ] avoid using PRODUCER_THREADS and CONSUMER_THREADS as #define (not sure of this change)



### Output of meta clustering 

* Output: uniprotID sstart send ML (Meta cluster label)
* Validation:

### Further exploring

* io streams are really slow:
  * check boost
  * read memory mapped files, scatter/gather I/O, etc/

* std algorithms accepts functions or structure as compare()? check