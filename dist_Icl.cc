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


#define BUFFER_SIZE 1000000  // size of internal write/read buffers in queuee 
#define CONSUMER_THREADS 8
#define MAX_qID 2353198020

// using namespace std;

typedef std::map<uint32_t, std::map<uint32_t, double> >  map2_t;
// typedef std::unordered_map<uint32_t, std::unordered_map<uint32_t, double > >  map2_t;
//---------------------------------------------------------------------------------------------------------
// GLOBAL VARIABLES
//---------------------------------------------------------------------------------------------------------

char * bufferA{NULL}, * bufferB{NULL};
uint64_t linesA{0}, linesB{0};

// Semaphores (TODO: cleaner implementation with CV)
std::array<sem_t, CONSUMER_THREADS> sem_write;
std::array<sem_t, CONSUMER_THREADS> sem_read;

// global flag to terminate consumers
bool done{0};
pthread_mutex_t done_lock = PTHREAD_MUTEX_INITIALIZER;

// array of map2_t for consumers
std::vector<map2_t> vec_maps(CONSUMER_THREADS);

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

struct MatchedPair
{
    uint32_t ID1;
    uint32_t ID2;
    double distance;

    MatchedPair(): ID1(0), ID2(0), distance(0) {}
    MatchedPair(uint32_t id1, uint32_t id2, double d): ID1(id1), ID2(id2), distance(d) {}
};

// Struct to pass to pthread functions
struct Queue
{
    uint32_t index;
    uint64_t fill;
    uint64_t fidx;
    uint64_t buffer_size;
    // using unique ptr to buffer. Good implementation?
    std::unique_ptr<MatchedPair []> write_buffer;
    std::unique_ptr<MatchedPair []> read_buffer;
    
                            
    Queue():index(0),fill(0), fidx(0), buffer_size(0){}
    Queue(uint32_t i, uint64_t f, uint64_t fi, uint64_t bs): 
    index(i), fill(f), fidx(fi), buffer_size(bs), write_buffer{new MatchedPair [bs]}, read_buffer{new MatchedPair [bs]}
    {}

};


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

    unsigned int total_producer_waittime{0};

    Queue *queues = (Queue*) qs;
    

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
                                               
                        if (queues[tidx].fidx  + 1 < queues[tidx].buffer_size)
                        {
                            queues[tidx].write_buffer[queues[tidx].fidx] = pair;
                            queues[tidx].fidx+=1;
                        }
                        else
                        {
                            // wait
                            auto t1_producer = std::chrono::high_resolution_clock::now();   
                            for(sem_t &sw : sem_write){sem_wait(&sw);}
                            auto t2_producer = std::chrono::high_resolution_clock::now();

                            unsigned int producer_waittime;
                            producer_waittime = std::chrono::duration_cast<std::chrono::milliseconds>
                                                (t2_producer-t1_producer).count();
                            total_producer_waittime += producer_waittime;
                            // std::cerr << "Producer waiting time: "<< producer_waittime << std::endl;

                            // swap
                            for (int i = 0; i < CONSUMER_THREADS; ++i)
                            {
                                queues[i].write_buffer.swap(queues[i].read_buffer);
                                // std::cerr << (double)queues[i].fidx/BUFFER_SIZE << " ";
                                // std::cerr << queues[i].index << " buffer: "<< queues[i].fidx << std::endl;
                                queues[i].fill = queues[i].fidx;
                                queues[i].fidx = 0;
                            }

                            // add missing element in new buffer
                            queues[tidx].write_buffer[queues[tidx].fidx] = pair;
                            queues[tidx].fidx+=1;
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
        // std::cerr << queues[i].index << " buffer: "<< queues[i].fidx << std::endl;
        queues[i].fill = queues[i].fidx;
        queues[i].fidx = 0;
    }
    // sem_post()
    for(sem_t &sr : sem_read){sem_post(&sr);}

    pthread_mutex_lock(&done_lock);
    std::cout << "total producer waittime: " << total_producer_waittime << std::endl;
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
        for (uint64_t i = 0; i < queue->fill; ++i)
        {
            vec_maps[queue->index][ queue->read_buffer[i].ID1 ][ queue->read_buffer[i].ID2 ] += queue->read_buffer[i].distance;
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
    std::array<Queue, CONSUMER_THREADS> queues;
    for (int i = 0; i < CONSUMER_THREADS; ++i)
    {
        queues[i] = Queue(i,0,0,BUFFER_SIZE);
    }
    

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
    for (int i = 0; i < CONSUMER_THREADS; ++i)
    {
        pthread_join(tids[i], NULL);    
    }
    
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
    std::cout << "done" << std::endl;
    delete[] bufferA;
    delete[] bufferB;


    auto t_output = std::chrono::high_resolution_clock::now();   
    auto total_waittime = std::chrono::duration_cast<std::chrono::milliseconds>
                        (t_output-t1_processing).count();
    std::cerr << "Total time (ms): "<< total_waittime << std::endl;
    
    
     
    return 0;

}






