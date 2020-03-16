#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <time.h>  
#include <iomanip>  

#include "smallca.hpp"

using namespace std;

// #include "fmt/format.h"

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



//_______________________________________________________________________________________________________//
/////////////////////////////////   M   A   I   N    //////////////////////////////////////////////////////


// USAGE:  ./a.out  input1 input2 recovery maxhours > output

int main(int argc, char** argv) {

    //time checking variables
    double max_hours; max_hours=atof(argv[4]);
    time_t tstart, tend; 
    double time_taken = 0;

    unsigned int recovery = atoi(argv[3]);
    cerr << "Found recovery point: " << recovery << endl;
   
    
    tstart = time(NULL); 

    // map<string, int> countingmap;  //map to be used to store the counts of matches between two clusters.
    map<unsigned int, map<unsigned int,unsigned int> >  countingmap;
    // key is LONG INT formed by [OLD qID*100+cl_idrel} qid_clidrel and it is a univoque identifier for a primary cluster (cl_idrel<=20 by definition on primarycl.cc)
    //map<int,int> clpops; //ma to be used to store the poulation of the clusters; to be used when computing distances. key is qID*100+cl_idrel


    // OPEN THE TWO FILES, READ BOTH'S FIST LINE, FIND SMALLEST sID    
    // input (contain clustered alignments); say "B" files (B as Block, eaach block contains data foro
    std::ifstream infileA (argv[1], std::ifstream::binary);
    std::ifstream infileB (argv[2], std::ifstream::binary);
    unsigned long int bytesA{0}, bytesB{0};
    char *bufferA, *bufferB;
    if (infileA)
    {
        infileA.seekg (0, infileA.end);
        bytesA = infileA.tellg();
        infileA.seekg (0, infileA.beg);
        bufferA = new char [bytesA];

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

    
    SmallCA * alA = (SmallCA *) bufferA;  //pointer to SmallCA to identify bufferA, i.e. the main file
    SmallCA * alB = (SmallCA *) bufferB; //pointer to SmallCA to bufferB, secondary file


    //scorri il file B finché O non c'è un match O scavalchi il valore che comanda (sidA>=sidB)
    unsigned int s0{0};
    unsigned long int posA{0}, posB{0};
    unsigned long int linesA = bytesA/sizeof(SmallCA);
    unsigned long int linesB = bytesB/sizeof(SmallCA);

        
    //scroll till the recovery checkpoint (both files A and B)
    while(posB<linesB && alB->sID<=recovery){++alB; ++posB;}
    while(posA<linesA && alA->sID<=recovery){++alA; ++posA;}


    while(posA<linesA && posB<linesB)
    {
        // std::cout << posA << '\t'<< posB << std::endl;
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

        //se = allora "pappappero"
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

                        if(pA->qID<=pB->qID)
                        {
                            ++countingmap[ pA->qID ][ pB->qID ];                            
                        } 
                        else
                        {
                            ++countingmap[ pB->qID ][ pA->qID ];                            
                        } 
                    }
                }

            }
            


            
        }   
        tend = time(NULL); 
        time_taken=difftime(tend, tstart)/3600.;
        if(time_taken>max_hours)  {cerr<< "\n ---- RECOVERY_ID "<< s0 << " time_taken " << time_taken << endl; goto stop;}
    }
       
    stop:
    
    // PRINT MAP
    auto outfile = std::fstream("outfile.bin", std::ios::out | std::ios::binary);
    const std::size_t lines = 100000;
    const std::size_t bytes = 3 * sizeof(unsigned int) * lines; // size in Bytes
    unsigned int *p = new unsigned int[3*lines];
    std::size_t j = 0;
    std::size_t i = 0;
    // std::size_t tot = 0;

    // auto myfile = std::fstream("outfile.binary", std::ios::out | std::ios::binary);
    for (auto itr_out = countingmap.cbegin(); itr_out != countingmap.cend(); ++itr_out) { 
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

    // cerr << tot << endl;
    delete[] p;
    delete[] bufferA;
    delete[] bufferB;
    outfile.close();

    
    tend = time(NULL); 
    time_taken=difftime(tend, tstart);
    cerr << "\nTOTAL time_taken " << time_taken << endl; 
    
     
    return 0;




}






