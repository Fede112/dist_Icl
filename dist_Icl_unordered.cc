#include <iostream>
#include <fstream>
#include <chrono>
#include <math.h>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>

#include <pthread.h>
#include <semaphore.h>

#include "smallca.h"
#include "normalization.h"
#include "concurrentqueue.h"


#define LOCAL_BUFFER_SIZE 100
#define CONSUMER_THREADS 8
#define MAX_qID 2353198020

using namespace moodycamel;
// using namespace std;

//---------------------------------------------------------------------------------------------------------
// GLOBAL TYPES
//---------------------------------------------------------------------------------------------------------

struct MatchedPair
{
    uint32_t ID1;
    uint32_t ID2;
    double distance;

    MatchedPair(): ID1(0), ID2(0), distance(0) {}
    MatchedPair(uint32_t id1, uint32_t id2, double d): ID1(id1), ID2(id2), distance(d) {}
};

typedef std::unordered_map<uint32_t, std::unordered_map<uint32_t, double > >  map_t;



//---------------------------------------------------------------------------------------------------------
// GLOBAL VARIABLES
//---------------------------------------------------------------------------------------------------------

char * bufferA{NULL}, * bufferB{NULL};
uint64_t linesA{0}, linesB{0};

// global flag to terminate consumers
bool done{0};
pthread_mutex_t doneLock = PTHREAD_MUTEX_INITIALIZER;

// consumer rank
int rankCount{0};
pthread_mutex_t rankLock = PTHREAD_MUTEX_INITIALIZER;

// array of map2_t for consumers
std::vector<map_t> vec_maps(CONSUMER_THREADS);

// qIDs_partition array for a balanced load among threads. You need (n-1) points to generate n partitions.
std::array <uint64_t, CONSUMER_THREADS-1> qIDs_partition; 

//---------------------------------------------------------------------------------------------------------
// ALIGNMENTS DISTANCE on the SEARCH
//---------------------------------------------------------------------------------------------------------
double dist(const SmallCA * i, const SmallCA * j){
    uint16_t hi, lo;
    double inte, uni;
    uint16_t istart, iend, jstart, jend;
    istart = i->sstart; iend = i->send; jstart = j->sstart; jend = j->send;
    //calculate intersection
    inte=0.0;
    hi=std::min(iend,jend);
    lo=std::max(istart,jstart);
    if(hi>lo) inte=hi-lo;
    //calculate union
    hi=std::max(iend,jend);
    lo=std::min(istart,jstart);
    uni=hi-lo;
    return (uni-inte)/uni;
}

//---------------------------------------------------------------------------------------------------------
// THREADS METHODS
//---------------------------------------------------------------------------------------------------------


void balanced_partition(std::array <uint64_t, CONSUMER_THREADS - 1> & array)
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
    ConcurrentQueue<MatchedPair> *queues = (ConcurrentQueue<MatchedPair>*) qs;
    
    // internal buffers

    uint64_t localBufferSize {LOCAL_BUFFER_SIZE}; 
    uint64_t localBufferIndex[CONSUMER_THREADS] = {0};
    MatchedPair localBuffer[CONSUMER_THREADS][localBufferSize] = {MatchedPair()};
    
    
    SmallCA * alA = (SmallCA *) bufferA;  //pointer to SmallCA to identify bufferA, i.e. the main file
    SmallCA * alB = (SmallCA *) bufferB; //pointer to SmallCA to bufferB, secondary file

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
            // std::cout << icount << std::endl;

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

                        auto qID1 = std::min(pA->qID, pB->qID);
                        auto qID2 = std::max(pA->qID, pB->qID);
                        auto norm = std::min(pA->qSize, pB->qSize);
                        auto pair = MatchedPair(qID1, qID2, 1./norm);
            
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
                        
                        localBuffer[tidx][localBufferIndex[tidx]] = pair;
                        ++localBufferIndex[tidx];
                        if (localBufferIndex[tidx] >= localBufferSize)
                        {
                            queues[tidx].enqueue_bulk(localBuffer[tidx], localBufferSize);
                            localBufferIndex[tidx]=0;
                        }
                        
                    }                    
                }
            }            
        }   
    }

    for (int i = 0; i < CONSUMER_THREADS; ++i)
    {
        for (int j = localBufferIndex[i]; j < LOCAL_BUFFER_SIZE; ++j)
        {
            localBuffer[i][j]=MatchedPair();
        }
        std::cout << localBufferIndex[i] << std::endl;
        queues[i].enqueue_bulk(localBuffer[i], localBufferIndex[i]);
    }
    

    pthread_mutex_lock(&doneLock);
    done = 1;
    pthread_mutex_unlock(&doneLock);
    pthread_exit(NULL);

}

void *consumer(void *q) 
{
    ConcurrentQueue<MatchedPair> * queue = (ConcurrentQueue<MatchedPair>*) q;

    pthread_mutex_lock(&rankLock);    
    int rank = rankCount;
    ++rankCount;
    pthread_mutex_unlock(&rankLock);
    // std::cerr << "Hi from consumer thread: " << rank << std::endl;

    while(1)
    {

        MatchedPair pairs[LOCAL_BUFFER_SIZE];

        if (queue->try_dequeue_bulk(pairs, LOCAL_BUFFER_SIZE))
            for (int i = 0; i < LOCAL_BUFFER_SIZE; ++i)
            {
                // uint64_t key = (((uint64_t)pairs[i].ID1) << 32 ) | pairs[i].ID2;
                // vec_maps[rank][ key ] += pairs[i].distance;
                vec_maps[rank][ pairs[i].ID1 ] [pairs[i].ID2]+= pairs[i].distance;
            }

        pthread_mutex_lock(&doneLock);
        if (done == 1)
        {
            while (queue->try_dequeue_bulk(pairs, LOCAL_BUFFER_SIZE))
            {
                for (int i = 0; i < LOCAL_BUFFER_SIZE; ++i)
                {
                    // uint64_t key = (((uint64_t)pairs[i].ID1) << 32 ) | pairs[i].ID2;
                    vec_maps[rank][ pairs[i].ID1 ] [pairs[i].ID2]+= pairs[i].distance;
                }
                
            }
            pthread_mutex_unlock(&doneLock);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&doneLock);

    }

}



//_______________________________________________________________________________________________________//
/////////////////////////////////   M   A   I   N    //////////////////////////////////////////////////////


// USAGE:  ./a.out  input1 input2 recovery maxhours > output



int main(int argc, char** argv) {


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

    
    // Initialize queues
    std::array<ConcurrentQueue<MatchedPair>, CONSUMER_THREADS> queues;

    // Define qIDs balanced partition per thread
    balanced_partition(qIDs_partition);

    //---------------------------------------------------------------------------------------------------------
    auto t1_processing = std::chrono::high_resolution_clock::now();   
    
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

    auto t_producer = std::chrono::high_resolution_clock::now();   
    for (int i = 0; i < CONSUMER_THREADS; ++i)
    {
        pthread_join(tids[i], NULL);    
    }

    auto t_consumer = std::chrono::high_resolution_clock::now();   
    auto diff_cons_prod = std::chrono::duration_cast<std::chrono::milliseconds>
                        (t_consumer-t_producer).count();
    std::cerr << "Difference consumer-producer (ms): "<< diff_cons_prod << std::endl;
     
    
    auto t2_processing = std::chrono::high_resolution_clock::now();   
    auto processing_waittime = std::chrono::duration_cast<std::chrono::milliseconds>
                        (t2_processing-t1_processing).count();
    std::cerr << "Processing time (ms): "<< processing_waittime << std::endl;
    //---------------------------------------------------------------------------------------------------------


    // PRINT MAP
    std::cout << "Writing to output... ";
    auto outfile = std::fstream("outfile.bin", std::ios::out | std::ios::binary);
    MatchedPair tmp;
    for (int tidx = 0; tidx < CONSUMER_THREADS; ++tidx)
    {
        for (auto itr_out = vec_maps[tidx].cbegin(); itr_out != vec_maps[tidx].cend(); ++itr_out) { 
            for (auto itr_in = itr_out->second.cbegin(); itr_in != itr_out->second.cend(); ++itr_in)
            {   
                tmp.ID1 = itr_out->first;
                tmp.ID2 = itr_in->first;
                tmp.distance = itr_in->second;
                outfile.write((char*)&tmp, sizeof(MatchedPair));
            }   
        } 
    }

    outfile.close();

    auto t3_output = std::chrono::high_resolution_clock::now();   
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>
                        (t3_output-t1_processing).count();
    std::cerr << "Total time (ms): "<< total_time << std::endl;

    delete[] bufferA;
    delete[] bufferB;

    
     
    return 0;

}






