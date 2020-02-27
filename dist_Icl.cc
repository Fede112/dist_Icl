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
using namespace std;

#include "fmt/format.h"



// Small Clustered Alignment structure (only essential data...)
struct SmallCA
{
    unsigned int qID;
    unsigned int sID;
    unsigned int sstart;
    unsigned int send;
    
    SmallCA(unsigned int q, unsigned int s, unsigned int ss, unsigned int se): qID(q), sID(s), sstart(ss), send (se) {}

};


//---------------------------------------------------------------------------------------------------------
// ALIGNMENTS DISTANCE on the SEARCH
//---------------------------------------------------------------------------------------------------------
double dist(SmallCA i, SmallCA j){
    unsigned int hi, lo;
    double inte, uni;
    unsigned int istart, iend, jstart, jend;
    istart= i.sstart; iend= i.send; jstart=j.sstart; jend=j.send;
    //calculate intersection
    inte=0;
    hi=min(iend,jend);
    lo=max(istart,jstart);
    if(hi>lo) inte=hi-lo;
    //calculate union
    hi=max(iend,jend);
    lo=min(istart,jstart);
    uni=hi-lo;
    return (uni-inte)/uni;
}


//read the line of an (OPENED) ifstream *file* into a ClusteredAlignment *ca*
int linetoSCA(ifstream& fileptr, SmallCA &SCA){
    // int is 4 bytes. long int is already 8 bytes
    unsigned int q, s, ss, se;

    string line;
    if(getline(fileptr, line)){
    istringstream iss(line); // make fileline into stream
    //read from stream
    iss >> q >> s >> ss >> se;
    //put in alignment B
        SCA=SmallCA(q, s, ss, se);
        return 0;}
    else if (fileptr.eof() ) return -1; //END OF FILE
    else return 1; // ERROR
}


//print a ClusteredAlignment TO STDOUT
void printSCA( SmallCA SCA) {

    cout << SCA.qID << " " << SCA.sID << " " << SCA.sstart << " " << SCA.send << endl;
    
}

// long long option_3(std::size_t bytes)
// {
//     std::vector<uint64_t> data = GenerateData(bytes);

//     std::ios_base::sync_with_stdio(false);
//     auto startTime = std::chrono::high_resolution_clock::now();
//     auto myfile = std::fstream("file.binary", std::ios::out | std::ios::binary);
//     myfile.write((char*)&data[0], bytes);
//     myfile.close();
//     auto endTime = std::chrono::high_resolution_clock::now();

//     return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
// }


//_______________________________________________________________________________________________________//
/////////////////////////////////   M   A   I   N    //////////////////////////////////////////////////////


// USAGE:  ./a.out  input1 input2 recovery maxhours > output

int main(int argc, char** argv) {

    //time checking variables
    double max_hours; max_hours=atof(argv[4]);
    time_t tstart, tend; 
    double time_taken = 0;

    int recovery = atoi(argv[3]);
    cerr << "Found recovery point: " << recovery << endl;
   
    
    tstart = time(NULL); 

    // map<string, int> countingmap;  //map to be used to store the counts of matches between two clusters.
    map<unsigned int, map<unsigned int,unsigned int> >  countingmap;
    // key is LONG INT formed by [OLD qID*100+cl_idrel} qid_clidrel and it is a univoque identifier for a primary cluster (cl_idrel<=20 by definition on primarycl.cc)
    //map<int,int> clpops; //ma to be used to store the poulation of the clusters; to be used when computing distances. key is qID*100+cl_idrel
    
    ifstream infile1, infile2; // input (contain clustered alignments); say "B" files (B as Block, eaach block contains data foro
    ifstream centers1, centers2; // input file containing, for each cluster, center and POP

    int checkA, checkB;
    
    ifstream * Afile;  //pointer to ifstream to identify Afile, i.e. the main file
    ifstream * Bfile; //pointer to ifstream to Bfile, secondary file
    
    string lineA, lineB;
    
    istringstream issA, issB;
    
    int s0=-1;
    
    SmallCA alA(0, 0, 0, 0);
    SmallCA alB(0, 0, 0, 0);
    // support variables for reading files
    
    // OPEN THE TWO B FILES, READ BOTH'S FIST LINE, FIND SMALLEST sID
    // open input1
    infile1.open (argv[1]);
    if (!infile1.is_open()) { cerr << "INFILE 1 NOT GOOD" << endl; return 1; }
    cerr << "\n \n    Input File:        " << argv[1] << endl;
    // open input2
    infile2.open (argv[2]);
    if (!infile2.is_open()) { cerr << "INFILE 2 NOT GOOD" << endl; return 1; }
    cerr << "\n \n    Input File:        " << argv[2] << endl;
    
    // the first guess is that A file will have smaller S; however, this will be checked.
    Afile = &infile1;
    Bfile = &infile2;

    //scroll till the recovery checkpoint (both files A and B)
    while(alB.sID<=recovery){
    checkB=linetoSCA(*Bfile,alB); if(checkB<0) break; else if(checkB>0){cerr << "(0)ERROR: one of the two files (B) cannot be read." << endl; return 2;} }
    while(alA.sID<=recovery){
    checkA=linetoSCA(*Afile,alA); if(checkA<0) break; else if(checkA>0){cerr << "(0)ERROR: one of the two files (A) cannot be read." << endl; return 2;} }


    
    checkA=linetoSCA(*Afile,alA);
    checkB=linetoSCA(*Bfile,alB);
    if(checkA<0 || checkB<0){cerr << "(1)ERROR: one of the two files has no lines." << endl; return 1;}
    if(checkA>0 || checkB>0){cerr << "(2)ERROR: one of the two files cannot be read." << endl; return 2;}
    
    
    //determine the main file
    if(alB.sID>alA.sID){ swap(alA,alB); swap(Afile,Bfile);  }
    
    //scorri il file B finché O non c'è un match O scavalchi il valore che comanda (sidA>=sidB)
    while(!infile1.eof() && !infile2.eof())
    {
        while(alB.sID<alA.sID){
            //cout << alA.sID << " " << alB.sID << endl;
            checkB=linetoSCA(*Bfile,alB);
            if(checkB<0) break; //file B ended.
            else if(checkB>0){cerr << "(3)ERROR: one of the two files cannot be read." << endl; return 2;}
            //cout << "--- "; printCA(alB);
        }
        
        //se > allora cambia
        if(alB.sID>alA.sID){ swap(alA,alB); swap(Afile,Bfile);  }
        //se = allora "pappappero"
        else if(alB.sID==alA.sID) 
        {
            s0 = alA.sID;
            //cout << "MATCH FOUND" << endl;
            //crea il vettore dove metterò tutti quelli con la stessa sID (1 x file)
            vector <SmallCA> vecA, vecB;
            //appendi i due allineamenti che ho già individuato
            // vecA.push_back(alA);
            // vecB.push_back(alB);
            
            do{ //appendi tutti gli altri che seguono con la stessa sID (sono sortati wrt sID apposta..) ---- A
                vecA.push_back(alA);
                checkA=linetoSCA(*Afile,alA);
                if(checkA<0) break; //file A ended.
                else if(checkA>0){cerr << "(4)ERROR: one of the two files cannot be read." << endl; return 2;}
                // if(alA.sID!=s0) break;
            }while(alA.sID==s0);
            
            do{ //appendi tutti gli altri che seguono con la stessa sID (sono sortati wrt sID apposta..) ---- B
                vecB.push_back(alB);
                checkB=linetoSCA(*Bfile,alB);
                if(checkB<0) break; //file B ended.
                else if(checkB>0){cerr << "(5)ERROR: one of the two files cannot be read." << endl; return 2;}
                // if(alB.sID!=s0) break;
            }while(alB.sID==s0);
            
            //compute distances on selected alignments with the same sID
            for(int i=0; i<vecA.size();++i)
            {
                for(int j=0; j<vecB.size();++j)
                {                    
                    if( dist(vecA[i],vecB[j]) < 0.2 ) 
                    {
                        if(vecA[i].qID<=vecB[j].qID)
                        {
                            ++countingmap[ vecA[i].qID ][ vecB[j].qID ];                            
                        } 
                        else
                        {
                            ++countingmap[ vecB[j].qID ][ vecA[i].qID ];                            
                        } 
                    }   
                }
            }

            tend = time(NULL); 
            time_taken=difftime(tend, tstart)/3600.;
            if(time_taken>max_hours)  {cerr<< "\n ---- RECOVERY_ID "<< s0 << " time_taken " << time_taken << endl; goto stop;}
        }
    }
    
    
    stop:
    // PRINT MAP
    // map<unsigned int, map<unsigned int, unsigned int>::iterator itr_out; 
    // map<unsigned int, unsigned int>::iterator itr_in; 




    const std::size_t lines = 100000;
    std::size_t bytes;
    bytes = 3 * sizeof(unsigned int) * lines; // size in Bytes
    std::size_t j = 0;

    // cout << "MB: " << bytes / 1024. / 1024. << endl;
    unsigned int *p = new unsigned int[3*lines];

    auto outfile = std::fstream("outfile.b", std::ios::out | std::ios::binary);


    // auto myfile = std::fstream("outfile.binary", std::ios::out | std::ios::binary);
    for (auto itr_out = countingmap.cbegin(); itr_out != countingmap.cend(); ++itr_out) { 
        for (auto itr_in = itr_out->second.cbegin(); itr_in != itr_out->second.cend(); ++itr_in, ++j)
        {
            p[j] = itr_out->first;
            p[j+1] = itr_in->first;
            p[j+2] = itr_in->second;
            if(j == lines-1)
            {
                outfile.write((char*)p, bytes);
                j = 0;
            }
        }
            // cout << itr_out->first << ' ' << itr_in->first << ' ' << itr_in->second << '\n'; 
    } 

    outfile.close();
    infile1.close();
    infile2.close();

    //int ndata;
    //ndata=vec.size();
    //cout << ndata << endl;

    //for(int i=0; i<vec.size();++i) cout  << vec[i].qID << " " << vec[i].sID << " " << vec[i].cl_idrel << endl;
    
    tend = time(NULL); 
    time_taken=difftime(tend, tstart);
    cerr << "\n TOTAL time_taken " << time_taken << endl; 
    
     
    return 0;




}






