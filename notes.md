# Notes for dist_Icl code

### Final pipeline

* Start from original 200 files ALREADY sorted by cluster id  (aX.txt where X i from 1 to 200)

* compute cluster size
   needs: convert aX.txt to aX.bin (OK);
   compute cluster sizes (TODO) - print binary output ( bX.bin ) which will have 1 extra column

* sort bX.bin wrt sID / merge 200 files in in 2 files sorted wrt qID
 (A) using Fede's programs (trying this)
 (B) convert bX.bin bX.txt (need proper program to do it); use  bash  ascii sort ; at the end reconvert everything <-(new conversion  programs with 1 extra column)
* compute dist



### TODO list

* istringstream really slow
* change normalized value to type double
* Improve code modularity

- C++ full migration. The code handles a combination of raw buffers with pointers arithmetics or std containers with assignment. The idea is to remove wrap the raw buffers with std containers. 

  - One key problem was loading a binary file into a vector. Apparently you can do it like this:

    ```cpp
    std::vector<double> buf(N / sizeof(double));// reserve space for N/8 doubles
    infile.read(reinterpret_cast<char*>(buf.data()), buf.size()*sizeof(double)); // or &buf[0] for C++98
    ```

    [reference](https://stackoverflow.com/questions/28707928/how-to-efficiently-read-a-binary-file-into-a-vector-c)

- Use uint32_t instead of unsigned int. You guarantee the 32 bits unsigned integer!

- Add more unit tests



### Further exploring

* io streams are really slow:
  * check boost
  * read memory mapped files, scatter/gather I/O, etc/