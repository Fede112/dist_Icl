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
#include <unordered_map>
#include <cstring> // memcpy



#include <memory>
#include <pthread.h>
#include <semaphore.h>

#include "smallca.h"
#include "normalization.h"

// size of production buffer* bufferA{NULL}s per thread
#define BUFFER_SIZE 50000000
#define MATCHING_THREADS 2
#define COUNTING_THREADS 3 // doesn't work for 1 CONSUMER THREAD 
#define MAX_qID 2353198020

using namespace std;


//---------------------------------------------------------------------------------------------------------
// GLOBAL VARIABLES
//---------------------------------------------------------------------------------------------------------

char * bufferA{NULL}, * bufferB{NULL};
unsigned int totalLinesA{0}, totalLinesB{0};

// queues mutex
pthread_mutex_t queuesLock[COUNTING_THREADS];
std::array<sem_t, COUNTING_THREADS> sem_write;

// Threads rank variable
uint32_t rankCount{0};
pthread_mutex_t rankLock = PTHREAD_MUTEX_INITIALIZER;

// array of map2_t for consumers
vector <NormalizedPairs> vec_frequency[COUNTING_THREADS];


// qIDs_partition array for a balanced load among threads. You need (n-1) points to generate n partitions.
std::array <unsigned int, COUNTING_THREADS-1> partqIDs{0}; 

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
// COUNTING_THREADS METHODS
//---------------------------------------------------------------------------------------------------------


// Struct to pass to pthread functions
struct Queue
{
    uint32_t index;
    uint64_t fill;
    uint64_t fidx;
    uint64_t bufferSize;
    // using unique ptr to buffer. Good implementation?
    std::unique_ptr<ClusterPairs []> buffer;
    
                            
    Queue():index(0),fill(0), fidx(0), bufferSize(0){}
    Queue(uint32_t i, uint64_t f, uint64_t fi, uint64_t bs): 
    index(i), fill(f), fidx(fi), bufferSize(bs), buffer{new ClusterPairs [bs]}
    {}

};

void queues_partition(std::array <unsigned int, COUNTING_THREADS - 1> & array)
// calculate the qIDs to equally distribute the pairs qID1-qID2 in each thread.
// It is an analytical expression based on the probability of finding a pair qID1-qID2, where qID1<qID2 always 
{
    for (int i = 0; i < COUNTING_THREADS-1; ++i)
    {
        array[i] = MAX_qID*(    1 - sqrt( 1 - (i+1)*((MAX_qID-1.)/(MAX_qID*COUNTING_THREADS)) )  );
    }
    return;
}


void files_partition(uint64_t (&partIndices)[MATCHING_THREADS+1][2])
// retrieve the indices of the partitions for bufferA and bufferB
{

    // partIndices[MATCHING_THREADS][NUM_FILES]

    // Index 0: bufferA and Index 1: bufferB
    SmallCA * alignment[2] = {(SmallCA*) bufferA, (SmallCA*) bufferB};
    
    uint64_t partSize[2] = {totalLinesA/MATCHING_THREADS, totalLinesB/MATCHING_THREADS};
    uint64_t partReminder[2] = {totalLinesA%MATCHING_THREADS, totalLinesB%MATCHING_THREADS};
    
    // uniform partition A
    partIndices[0][0] = 0;
    partIndices[0][1] = 0;
    for (uint32_t i = 1; i <=  MATCHING_THREADS; ++i)
    {
        for (uint32_t f = 0; f < 2; ++f)
        {
            partIndices[i][f] = partSize[f] + partIndices[i-1][f];
            if (i <= partReminder[f]) {partIndices[i][f] +=1;}      
        }
    }

    // shift to match sID
    for (uint32_t i = 1; i <  MATCHING_THREADS; ++i)
    {
        // we use bufferA for picking the reference sID
        auto sValue = (alignment[0] + partIndices[i][0])->sID;
        
        for (uint32_t f = 0; f < 2; ++f)
        {
            auto start = std::lower_bound(alignment[f] + partIndices[i-1][f], alignment[f] + partIndices[i+1][f], sValue, compare_sID());
            partIndices[i][f] -= (alignment[f] + partIndices[i][f]) - start; 
        }
        
    }
    return;
}

void *matching_clusters(void *qs) 
{
    Queue *queues = (Queue*) qs;


    pthread_mutex_lock(&rankLock);
    int rank = rankCount;
    ++rankCount;
    pthread_mutex_unlock(&rankLock);
    
    // std::cout << "Hi from thread: " << rank << std::endl;

    // find the partition of file A and B to analize
    uint64_t partIndices[MATCHING_THREADS+1][2] = {0};    
    files_partition(partIndices);

    SmallCA * alA = (SmallCA *) bufferA  + partIndices[rank][0];  //pointer to SmallCA to identify bufferA, i.e. the main file
    SmallCA * alB = (SmallCA *) bufferB  + partIndices[rank][1]; //pointer to SmallCA to bufferB, secondary file

    uint64_t linesA = partIndices[rank+1][0] - partIndices[rank][0];
    uint64_t linesB = partIndices[rank+1][1] - partIndices[rank][1];

    uint64_t posA{0}, posB{0};
    uint32_t s0{0};


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
                        // int norm{2};
                        auto qID1 = min(pA->qID, pB->qID);
                        auto qID2 = max(pA->qID, pB->qID);
                        auto norm = min(pA->qSize, pB->qSize);
                        
                        // decide to which queue the pair goes
                        int ctId = COUNTING_THREADS - 1;
                        for (int i = 1; i < COUNTING_THREADS; ++i)
                        {
                            if(qID1 < partqIDs[i-1])
                            {
                                ctId = i-1;
                                break;
                            }
                        }

                        // get mutex in position ctId 
                        pthread_mutex_lock(&queuesLock[ctId]);
                        // sem_wait(&sem_write[ctId]);

                        if (queues[ctId].fidx  + 1 < queues[ctId].bufferSize)
                        {
                            queues[ctId].buffer[queues[ctId].fidx].ID = (((uint64_t)qID1) << 32 ) | qID2;
                            queues[ctId].buffer[queues[ctId].fidx].norm = norm;
                            queues[ctId].fidx++;
                        }
                        // free mutex in position ctId
                        pthread_mutex_unlock(&queuesLock[ctId]);
                        // sem_post(&sem_write[ctId]); 
                    }
                }
            }
        }
    }

    // std::cout << "rank: " << rank << '\t' << internal_count << std::endl;
    pthread_exit(NULL);
}

void *counting(void *q) 
{
    Queue * queue = (Queue*) q;
    // std::cout << "Hi from counting thread: " << queue->index << std::endl;

    auto tmp = (unsigned char *) queue->buffer.get();
    radix_sort(tmp, queue->fidx, sizeof(ClusterPairs), 8, 0);
    
    
    vec_frequency[queue->index].reserve(queue->fidx);
    frequency(queue->buffer.get(), queue->fidx, vec_frequency[queue->index]);

    pthread_exit(NULL);


}


//_______________________________________________________________________________________________________//
/////////////////////////////////   M   A   I   N    //////////////////////////////////////////////////////


// USAGE:  ./a.out  input1 input2 recovery maxhours > output

int main(int argc, char** argv) {


    // Initialize semaphores
    // for(sem_t &sw : sem_write){sem_init(&sw, 0, 1);}
    // for(sem_t &sr : sem_read){sem_init(&sr, 0, 0);}

    //time checking variables
    time_t tstart, tend; 
    double time_taken = 0;

   
    
    tstart = time(NULL); 

    // key is LONG INT formed by [OLD qID*100+cl_idrel} qid_clidrel and it is a univoque identifier for a primary cluster (cl_idrel<=20 by definition on primarycl.cc)


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
        totalLinesA = bytesA/sizeof(SmallCA);

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
        totalLinesB = bytesB/sizeof(SmallCA);

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


    // SmallCA * alA = (SmallCA *) bufferA;  //pointer to SmallCA to identify bufferA, i.e. the main file
    // SmallCA * alB = (SmallCA *) bufferB; //pointer to SmallCA to bufferB, secondary file

    
    // Define qIDs balanced partition per thread
    queues_partition(partqIDs);

    // Initialize queues
    std::array<Queue, COUNTING_THREADS> queues;
    for (int i = 0; i < COUNTING_THREADS; ++i)
    {
        queues[i] = Queue(i,0,0,BUFFER_SIZE);
    }
    
    // Initialize mutex for each queue
    for (int i = 0; i < COUNTING_THREADS; i++)
        pthread_mutex_init(&queuesLock[i], NULL);

    for(sem_t &sw : sem_write){sem_init(&sw, 0, 1);}

    
    // Matching Threads: each thread analyses a segment of (fileA, fileB) 
    // and fills the different queues used later by Counting Threads
    pthread_t tids_m[MATCHING_THREADS];
    for (int i = 0; i < MATCHING_THREADS; ++i)
    {
        int *rank = (int *) malloc(sizeof(*rank));
        *rank = i;
        // Create attributes
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        auto thread_err = pthread_create(&tids_m[i], &attr, matching_clusters, &queues);
        if (thread_err != 0)
                printf("\nCan't create thread :[%s]", strerror(thread_err));
    } 



    // Join matching threads 
    for (int i = 0; i < MATCHING_THREADS; ++i)
    {
        pthread_join(tids_m[i], NULL);    
    }

    std::cout << queues[0].fidx << '\t' << queues[1].fidx << '\t' << queues[2].fidx << std::endl;




    
    // Consumer COUNTING_THREADS
    pthread_t tids_c[COUNTING_THREADS];
    for (int i = 0; i < COUNTING_THREADS; ++i)
    {
        // Create attributes
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tids_c[i], &attr, counting, &queues[i]);
    } 



    // Join counting threads
    for (int i = 0; i < COUNTING_THREADS; ++i)
    {
        pthread_join(tids_c[i], NULL);    
    }
    
    tend = time(NULL); 
    time_taken=difftime(tend, tstart);
    cerr << "\nProcessing time_taken " << time_taken << endl; 

    // PRINT OUTPUT
    std::cout << "Writing to output: " << std::endl;
    auto outfile = std::fstream("outfile.bin", std::ios::out | std::ios::binary);
    for (int tid = 0; tid < COUNTING_THREADS; ++tid)
    {
        for (auto & elem: vec_frequency[tid]) 
        {
            outfile.write((char*)&elem, sizeof(ClusterPairs));
        }
    }

    outfile.close();
    delete[] bufferA;
    delete[] bufferB;

    
    // tend = time(NULL); 
    // time_taken=difftime(tend, tstart);
    // cerr << "\nTOTAL time_taken " << time_taken << endl; 
    
     
    return 0;

}






