#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <time.h>  
#include <chrono>
#include <iomanip>  

#include <memory>
#include <pthread.h>
#include <semaphore.h>

#include "smallca.h"

// size of production buffer* bufferA{NULL}s per thread
#define BUFFER_SIZE 6000000
#define CONSUMER_THREADS 3 // doesn't work for 1 CONSUMER THREAD 
#define MAX_qID 2353198020

using namespace std;

typedef std::map<unsigned int, std::map<unsigned int,unsigned int> >  map2_t;

//---------------------------------------------------------------------------------------------------------
// GLOBAL VARIABLES
//---------------------------------------------------------------------------------------------------------

char * bufferA{NULL}, * bufferB{NULL};
unsigned int linesA{0}, linesB{0};

// Semaphores (TODO: cleaner implementation with CV)
std::array<sem_t, CONSUMER_THREADS> sem_write;
std::array<sem_t, CONSUMER_THREADS> sem_read;

// global flag to terminate consumers
unsigned int done{0};
pthread_mutex_t done_lock = PTHREAD_MUTEX_INITIALIZER;

// array of map2_t for consumers
std::vector<map2_t> vec_maps(CONSUMER_THREADS);

// qIDs_partition array for a balanced load among threads. You need (n-1) points to generate n partitions.
std::array <unsigned int, CONSUMER_THREADS-1> qIDs_partition{0}; 

//---------------------------------------------------------------------------------------------------------
// ALIGNMENTS DISTANCE on the SEARCH
//---------------------------------------------------------------------------------------------------------
double dist(const SmallCA * i, const SmallCA * j){
    unsigned int hi, lo;
    double inte, uni;
    unsigned short istart, iend, jstart, jend;
    istart = i->sstart; iend = i->send; jstart = j->sstart; jend = j->send;
    //calculate intersection
    inte=0.0;
    hi=min(iend,jend);
    lo=max(istart,jstart);
    if(hi>lo) inte=hi-lo;
    //calculate union
    hi=max(iend,jend);
    lo=min(istart,jstart);
    uni=hi-lo;
    return (uni-inte)/uni;
}

//---------------------------------------------------------------------------------------------------------
// THREADS METHODS
//---------------------------------------------------------------------------------------------------------


// Struct to pass to pthread functions
struct Queue
{
    unsigned int index;
    unsigned int fill;
    unsigned int fidx;
    unsigned int buffer_size;
    // using unique ptr to buffer. Good implementation?
    std::unique_ptr<unsigned int []> write_buffer;
    std::unique_ptr<unsigned int []> read_buffer;
    
                            
    Queue():index(0),fill(0), fidx(0), buffer_size(0){}
    Queue(unsigned int i, unsigned int f, unsigned int fi, unsigned int bs): 
    index(i), fill(f), fidx(fi), buffer_size(bs), write_buffer{new unsigned int [bs]}, read_buffer{new unsigned int [bs]}
    {}

};

void balanced_partition(std::array <unsigned int, CONSUMER_THREADS - 1> & array)
// calculate the qIDs to equally distribute the pairs qID1-qID2 in each thread.
// It is an analytical expression based on the probability of finding a pair qID1-qID2, where qID1<qID2 always 
{
    for (int i = 1; i < CONSUMER_THREADS; ++i)
    {
        array[i-1] = MAX_qID*(    1 - sqrt( 1 - i*((MAX_qID-1.)/(MAX_qID*CONSUMER_THREADS)) )  );
    }
    return;
}



void *producer(void *qs) 
{
    Queue *queues = (Queue*) qs;
    // std::cerr << "Hi from producer thread " << std::endl;
    

    SmallCA * alA = (SmallCA *) bufferA;  //pointer to SmallCA to identify bufferA, i.e. the main file
    SmallCA * alB = (SmallCA *) bufferB; //pointer to SmallCA to bufferB, secondary file

    unsigned long int posA{0}, posB{0};
    unsigned int s0{0};

    while(posA<linesA && posB<linesB)
    {
        while(posB<linesB && alB->sID<alA->sID)
        {
            ++alB;
            ++posB;
        }
        while(posA<linesA && alB->sID>alA->sID)
        {
            ++alA;
            ++posA;
        }

        if(alB->sID == alA->sID) 
        {
            s0 = alA->sID;
            const SmallCA * init_subA = alA;
            const SmallCA * init_subB = alB;
            
            while(posA<linesA && alA->sID == s0 ) //cerca tutti gli altri che seguono con la stessa sID (sono sortati wrt sID apposta..) ---- A
            {
                ++alA;
                ++posA;
            }

            while(posB<linesB && alB->sID == s0 ) //cerca tutti gli altri che seguono con la stessa sID (sono sortati wrt sID apposta..) ---- B
            {
                ++alB;
                ++posB;
            }            
            
            //compute distances on selected alignments with the same sID
            for (auto pA = init_subA; pA < alA; ++pA)
            {
                for (auto pB = init_subB; pB < alB; ++pB)
                {
                    if( dist(pA,pB) < 0.2 ) 
                    {

                        auto qID1 = min(pA->qID, pB->qID);
                        auto qID2 = max(pA->qID, pB->qID);
            
                        // decide to which queue the pair goes
                        int tidx = CONSUMER_THREADS - 1;

                        
                        for (int i = 1; i < CONSUMER_THREADS; ++i)
                        {
                            if(qID1 < qIDs_partition[i-1])
                            {
                                tidx = i-1;
                                break;
                            }
                        }

                        Pthread_mutex_lock(&mutex);
                        if (swap == 1)
                            ++prod_waiting;
                            if (prod_waiting == PROC_THREADS -1)
                                // signal swap producer to continue
                            Pthread_cond_wait(&cond, &mutex);
                            --prod_waiting;
                            // unlock mutex


                        // get mutex in position tidx                       
                        if (queues[tidx].fidx  + 1 < queues[tidx].buffer_size)
                        {
                            queues[tidx].write_buffer[queues[tidx].fidx] = qID1;
                            queues[tidx].write_buffer[queues[tidx].fidx+1] = qID2;
                            queues[tidx].fidx+=2;
                        // free mutex
                        }
                        else
                        {
                            // free index_lock

                            // try swap_lock
                            
                            // if trywap fails => enters producer barrier
                            //
                            // else => swap
                            Pthread_mutex_lock(&mutex);
                            swap = 1; // global variable to indicate we are swapping
                            Pthread_mutex_unlock(&mutex);

                            // producer barrier
                            Pthread_cond_wait(&cond, &mutex)

                            // only one grabs swap_lock (if statement, rest skips step)

                            // cerr << "Producer: swapping " << tidx << " with " << queues[tidx].fidx << endl; 
                            // they all wait for the writers semaphore or cv
                            // wait
                            auto t1_producer = std::chrono::high_resolution_clock::now();   
                            for(sem_t &sw : sem_write){sem_wait(&sw);}
                            auto t2_producer = std::chrono::high_resolution_clock::now();


                            // grab swap_lock
                            // if swap flag 0: swap

                            unsigned int producer_waittime;
                            producer_waittime = std::chrono::duration_cast<std::chrono::milliseconds>
                                                (t2_producer-t1_producer).count();
                            cerr << "Producer waiting time: "<< producer_waittime << endl;

                            // swap
                            for (int i = 0; i < CONSUMER_THREADS; ++i)
                            {
                                queues[i].write_buffer.swap(queues[i].read_buffer);
                                cerr << (double)queues[i].fidx/BUFFER_SIZE << " ";
				                if(i==CONSUMER_THREADS) cerr << endl;
                                queues[i].fill = queues[i].fidx;
                                queues[i].fidx = 0;
                            }
                            cerr << '\n';

                            // free swap lock
                            // else add 

                            // get mutex in position index
                            // add missing element in new buffer
                            queues[tidx].write_buffer[queues[tidx].fidx] = qID1;
                            queues[tidx].write_buffer[queues[tidx].fidx+1] = qID2;
                            queues[tidx].fidx+=2;
                            // sem_post()
                            for(sem_t &sr : sem_read){sem_post(&sr);}
                        }
                    }                    
                }
            }            
        }   
    }

    // wait
    for(sem_t &sw : sem_write){sem_wait(&sw);}

    // swap
    for (int i = 0; i < CONSUMER_THREADS; ++i)
    {
        queues[i].write_buffer.swap(queues[i].read_buffer);
        cerr << queues[i].index << " buffer: "<< queues[i].fidx << endl;
        queues[i].fill = queues[i].fidx;
        queues[i].fidx = 0;
    }
    // sem_post()
    for(sem_t &sr : sem_read){sem_post(&sr);}

    pthread_mutex_lock(&done_lock);
    done = 1;
    pthread_mutex_unlock(&done_lock);
    pthread_exit(NULL);

}

void *consumer(void *q) 
{
    Queue * queue = (Queue*) q;
    // std::cerr << "Hi from consumer thread: " << queue->index << std::endl;

    while(1)
    {
        // wait for sem_read
        sem_wait(&sem_read[queue->index]);

        // read entire buffer
        for (unsigned int i = 0; i < queue->fill; i=i+2)
        {
            ++vec_maps[queue->index][ queue->read_buffer[i] ][ queue->read_buffer[i+1] ];
        }

        // sem_post write
        sem_post(&sem_write[queue->index]);


        // exit routine condition
        pthread_mutex_lock(&done_lock);
        if (done == 1)
        {
            pthread_mutex_unlock(&done_lock);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&done_lock);

    }

}



//_______________________________________________________________________________________________________//
/////////////////////////////////   M   A   I   N    //////////////////////////////////////////////////////


// USAGE:  ./a.out  input1 input2 recovery maxhours > output

int main(int argc, char** argv) {


    // Initialize semaphores
    for(sem_t &sw : sem_write){sem_init(&sw, 0, 1);}
    for(sem_t &sr : sem_read){sem_init(&sr, 0, 0);}

    //time checking variables
    // double max_hours; max_hours=atof(argv[4]);
    time_t tstart, tend; 
    double time_taken = 0;

    // unsigned int recovery = atoi(argv[3]);
    // cerr << "Found recovery point: " << recovery << endl;
   
    
    tstart = time(NULL); 

    // map<string, int> countingmap;  //map to be used to store the counts of matches between two clusters.
    // map<unsigned int, map<unsigned int,unsigned int> >  countingmap;
    // key is LONG INT formed by [OLD qID*100+cl_idrel} qid_clidrel and it is a univoque identifier for a primary cluster (cl_idrel<=20 by definition on primarycl.cc)
    //map<int,int> clpops; //ma to be used to store the poulation of the clusters; to be used when computing distances. key is qID*100+cl_idrel


    // OPEN THE TWO FILES, READ BOTH'S FIST LINE, FIND SMALLEST sID    
    // input (contain clustered alignments); say "B" files (B as Block, eaach block contains data foro
    std::ifstream infileA (argv[1], std::ifstream::binary);
    std::ifstream infileB (argv[2], std::ifstream::binary);
    unsigned long int bytesA{0}, bytesB{0};
    if (infileA)
    {
        infileA.seekg (0, infileA.end);
        bytesA = infileA.tellg();
        infileA.seekg (0, infileA.beg);
        bufferA = new char [bytesA];
        linesA = bytesA/sizeof(SmallCA);

        std::cerr << "Reading " << bytesA << " characters... ";
        // read data as a block:
        infileA.read (bufferA,bytesA);
        std::cerr << "all characters read successfully from " << argv[1] << "\n";
    }
    else
    {
      std::cout << "error: only " << infileA.gcount() << " could be read";
      return 1;
    }
    infileA.close();
    
    if (infileB)
    {
        infileB.seekg (0, infileB.end);
        bytesB = infileB.tellg();
        infileB.seekg (0, infileB.beg);
        bufferB = new char [bytesB];
        linesB = bytesB/sizeof(SmallCA);

        std::cerr << "Reading " << bytesB << " characters... ";
        // read data as a block:
        infileB.read (bufferB,bytesB);
        std::cerr << "all characters read successfully from " << argv[2] << "\n";
    }
    else
    {
      std::cout << "error: only " << infileB.gcount() << " could be read";
      return 1;
    }
    infileB.close();

        
    //scroll till the recovery checkpoint (both files A and B)
    // while(posB<linesB && alB->sID<=recovery){++alB; ++posB;}
    // while(posA<linesA && alA->sID<=recovery){++alA; ++posA;}

    
    // Initialize queues
    std::array<Queue, CONSUMER_THREADS> queues;
    for (int i = 0; i < CONSUMER_THREADS; ++i)
    {
        queues[i] = Queue(i,0,0,BUFFER_SIZE);
    }
    
    // Define qIDs balanced partition per thread
    balanced_partition(qIDs_partition);

    // Producer thread
    pthread_t tprod;
    pthread_create(&tprod, NULL, producer, &queues);
    
    // Consumer threads
    pthread_t tids[CONSUMER_THREADS];
    for (int i = 0; i < CONSUMER_THREADS; ++i)
    {
        // Create attributes
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tids[i], &attr, consumer, &queues[i]);
    } 

    /* Here main thread could do normalization calculation 
        while cluster distance is calculated in child threads */


    // Join threads when finished
    pthread_join(tprod, NULL);    
    for (int i = 0; i < CONSUMER_THREADS; ++i)
    {
        pthread_join(tids[i], NULL);    
    }
    


    // PRINT MAP
    std::cout << "Writing to output: " << std::endl;
    auto outfile = std::fstream("outfile.bin", std::ios::out | std::ios::binary);
    const std::size_t lines = 100000;
    const std::size_t bytes = 3 * sizeof(unsigned int) * lines; // size in Bytes
    unsigned int *p = new unsigned int[3*lines];
    for (int tidx = 0; tidx < CONSUMER_THREADS; ++tidx)
    {
        std::size_t j = 0;
        std::size_t i = 0;
        // std::size_t tot = 0;

    
        for (auto itr_out = vec_maps[tidx].cbegin(); itr_out != vec_maps[tidx].cend(); ++itr_out) { 
            for (auto itr_in = itr_out->second.cbegin(); itr_in != itr_out->second.cend(); ++itr_in, ++j, ++i)
            {   
                // ++tot;
                p[3*j] = itr_out->first;
                p[3*j+1] = itr_in->first;
                p[3*j+2] = itr_in->second;
                if(j == lines-1)
                {
                    outfile.write((char*)p, bytes);
                    j = -1; // so that j starts from 0 in the next iteration
                }
            }   
        } 
        outfile.write((char*)p, 3*sizeof(unsigned int)*j);
    }

    // cerr << tot << endl;
    outfile.close();
    delete[] p;
    delete[] bufferA;
    delete[] bufferB;

    
    tend = time(NULL); 
    time_taken=difftime(tend, tstart);
    cerr << "\nTOTAL time_taken " << time_taken << endl; 
    
     
    return 0;

}






