#include <stdlib.h>

#include "Riostream.h"
#include "TROOT.h"
#include "TFile.h"
#include "TNetFile.h"
#include "TWebFile.h"
#include "TSSLSocket.h"
#include "TRandom.h"
#include "TTree.h"
#include "TTreeCache.h"
#include "TTreePerfStats.h"
#include "TBranch.h"
#include "TClonesArray.h"
#include "TStopwatch.h"
#include "TTreeCacheUnzip.h"
#include "TEnv.h"

#include <iostream>
#include <fstream>
#include <sstream>

void diskSt(long long * diskStat, string fn){
  
    // first finding base directory for the file
    int pos=fn.find('/',2);
    string dis;
    dis=fn.substr(0, pos);
    cout<<"\nmount point from fn: "<<dis<<endl;
    
    // finding which device has this mount
    string na, nb, nc, nd;
    int i1;    
    ifstream mounts ("/proc/mounts", ios::in);
    if (mounts.is_open()){
        while(!mounts.eof()){
            mounts>>na>>nb>>nc>>nd>>i1>>i1;
            if (nb.compare(dis)==0)  {
                cout<<na<<"\t"<<nb<<"\t";
                if (na.substr(0,5).compare("/dev/")==0)
                    na=na.substr(5);
                cout<<na<<endl;    
                break;
            }
        }
    }
    
	ifstream file ("/proc/diskstats", ios::in);
	if (file.is_open()){
		while(!file.eof()){
			string name="";
			file>>i1>>i1>>name;
            // cout<<name<<"\t";
            for (int c=0;c<11;c++) {
                file>>diskStat[c]; 
                // cout<<diskStat[c]<<"\t";
            }
            // cout<<endl;    
            if (!name.compare(na)) {
                // for (int c=0;c<11;c++) cout<<diskStat[c]<<"\t"; cout<<endl;
                break;
			    };
		} 
	}
	file.close();
}

void getMemoryInfo(){
    ifstream file ("/proc/self/status", ios::in);
    int i;
	if (file.is_open()){
		while(!file.eof()){
			string name="";
			file>>name;
            // cout<<name<<"\t";
            if (!name.compare("VmPeak:")) {file>>i;cout<<"VMEM="<<i<<endl;}
            if (!name.compare("VmRSS:")) {file>>i;cout<<"RSS="<<i<<endl;file.close();return;}	    
		} 
	}
	file.close();
}

void getCPUusage(){
    FILE *pf;
    char data[48];
    pf=popen("uptime |awk -F'average:' '{ print $2}'  | sed s/,//g | awk '{ print $3}' ","r");
    if(!pf){
        fprintf(stderr, "Could not open pipe for output.\n");
        return;
    }
    fgets(data, 48 , pf);
    cout<<"CPUSTATUS="<<data<<endl;
}

void netSt(long long *netStat){
    system("netstat -in > nstat.txt");
    ifstream file("nstat.txt", ios::in);
    int i;netStat[0]=0;netStat[1]=0;
	if (file.is_open()){
		while(!file.eof()){
			string name="";
			file>>name;
            if (!name.compare("eth0")) {
                file>>i>>i>>netStat[0]>>i>>i>>i>>netStat[1]; 
                break;
			    }
		}
	}
	
    system("rm nstat.txt");
	file.close();
}

int readDPMWebDav(string fn, string trname, int percentage, float TTC, string branchesToBeRead = "", string proxyfn="", string certdir="", int endevent=-1){ 
    
    //gEnv->SetValue("TFile.AsyncPrefetching", 1);

    TStopwatch timer;
    Double_t nb = 0;	
    bool checkHDD=true;
    long long diskS1[11];
    long long diskS2[11];
    long long netSt1[2];
    long long netSt2[2];
    
    string str2("dcap://");
    size_t found;
    found=fn.find(str2);
    if (found!=string::npos){
          cout << "dcap - no disk accesses available." << endl;
          checkHDD=false;
		  cout<<"FILESYSTEM='dcap'"<<endl;
    }
	str2="root://";
	found=fn.find(str2);
    if (found!=string::npos){
          cout << "xrootd - no disk accesses available." << endl;
          checkHDD=false;
          cout<<"FILESYSTEM='root'"<<endl;
    }
    
    if (checkHDD){
        char buf[2048];
        int count = readlink(fn.c_str(), buf, sizeof(buf));
        if (count >= 0) {
           buf[count] = '\0';
           printf("%s -> %s\n", fn.c_str(), buf);
		   fn = string(buf);
        }
    
        diskSt(diskS1, fn);
    }
 
    netSt(netSt1);
    TFile *f ;

    // Set up the Davix auth, for when Davix will be available
    gEnv->SetValue("Davix.GSI.CACheck", "n");  
    gEnv->SetValue("Davix.GSI.CAdir", "/etc/grid-security/certificates/");
    if (certdir.size() > 2) {
        cout << "Setting Davix.GSI.CAdir to the content of certdir" << endl;
        gEnv->SetValue("Davix.GSI.CAdir", certdir.c_str());
    }

    if (proxyfn.size() > 2) gEnv->SetValue("Davix.GSI.UserProxy", proxyfn.c_str());
    gEnv->SetValue("Davix.GSI.GridMode", "y");
//    gEnv->SetValue("Davix.Debug", 0);

    // Also setup the xrootd auth, in the case xrootd is used
    gEnv->SetValue("XSec.GSI.UserProxy", proxyfn.c_str());
   
    cout << "Opening '" << fn << "'" << endl; 
    f = TFile::Open(fn.c_str());
    if (!f) {
      cout << "TFile::Open failed. Trying again with TWebFile...";
      f = new TWebFile(fn.c_str());
      }


    if (!f) {
      cout << "File open failed... every hope is vain. Surrender.";
      return -1;
    }

    TTree *tree = (TTree*)f->Get(trname.c_str());
    TTreeCache::SetLearnEntries(1);

    Int_t nentries = (Int_t)tree->GetEntries();
    if (endevent > 0 ) nentries = endevent; 
    cout << "nentries " << nentries << endl;

	float toRead= (float) nentries * percentage/100;
	int* randoms = new int[nentries];
	
	tree->SetCacheSize((int)(TTC*1024*1024));

    if (branchesToBeRead.length()>0) {
        tree->SetBranchStatus("*",0);
        ifstream file (branchesToBeRead.c_str(), ios::in);
    	if (file.is_open()){
    		while(!file.eof()){
    			string name="";
    			file>>name;
                tree->SetBranchStatus(name.c_str(),1);
                tree->AddBranchToCache(name.c_str());
                // cout<<name<<"\n";   
    		} 
    	}
    	file.close();
    }
    else {
        tree->AddBranchToCache("*",kTRUE);
    }
    
    tree->StopCacheLearningPhase();
    //TTreeCache::SetLearnEntries(1);
    TTreePerfStats *ps= new TTreePerfStats("ioperf",tree);
    
    for (int i=0;i<nentries;i++) {
      if (gRandom->Rndm(1)<((float) percentage/100)) {
	randoms[i]=1;
      } else randoms[i]=0;
    }
    
    
    	
    timer.Start();
    cout << "Starting event loop " << endl ;
    
    for (Int_t ev = 0; ev < nentries; ev++) {
      //	  if (ev%10 == 0 ){
      //	    cout << "processed" << ev << " entries" << endl; 
      //	  }
      
      if (randoms[ev]==0) continue;
		nb += tree->GetEntry(ev);
    }
    timer.Stop();
	
    nb/=1024.0/1024.0;

    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
	
    tree->PrintCacheStats();
    //ps->SaveAs("perf.root");
    ps->Print(); 

    cout<<"TOTALSIZE="<<tree->GetTotBytes()<<endl;
    cout<<"ZIPSIZE="<<tree->GetZipBytes()<<endl;
    cout<<"FILENAME='"<<fn<<"'"<<endl;
    cout<<"EVENTS="<<nentries<<endl;
    cout<<"WALLTIME="<<rtime<<endl;
    cout<<"CPUTIME="<<ctime<<endl;
    cout<<"CACHESIZE="<<TTC*1024*1024<<endl;
    cout<<"Read from tree: " <<nb<<endl;
    cout<<"ROOTBYTESREAD=" <<f->GetBytesRead()<<endl;
    cout<<"ROOTREADS="<<f->GetReadCalls()<<endl;
    cout<<"ROOTVERSION="<<gROOT->GetSvnRevision()<<endl;
    cout<<"ROOTBRANCH='"<<gROOT->GetSvnBranch()<<"'"<<endl;
    
    f->Close();
    delete f;
    getMemoryInfo();    
    getCPUusage();
    // cout << nentries<< " events"<< endl;
    // cout << "Compression level: "<< f->GetCompressionLevel()<<endl;
	//printf("file compression factor = %f\n",f->GetCompressionFactor());
	
    if (checkHDD){
      diskSt(diskS2, fn);
      cout<<"HDDREADS="<<diskS2[0]-diskS1[0]<<endl;
      cout<<"reads merged:  "<<diskS2[1]-diskS1[1]<<endl;
      cout<<"reads sectors: "<<diskS2[2]-diskS1[2]<<"   in Mb: "<<(diskS2[2]-diskS1[2])/2048<<endl;
      cout<<"HDDTIME="<<diskS2[3]-diskS1[3]<<endl;
    }

    
    netSt(netSt2);
    cout<<"ETHERNETIN="<<netSt2[0]-netSt1[0]<<endl;
    cout<<"ETHERNETOUT="<<netSt2[1]-netSt1[1]<<endl;
    
    
    return 0;
}



int main(int argc, char** argv){
	//davix_set_log_level(15);
	if(argc < 2){
		std::cout << "Usage: " << argv[0] <<" [url] " << std::endl;
		return 1;
	}
	return readDPMWebDav(argv[1], "physics", 100, 30);


}
