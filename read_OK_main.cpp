#include <cstdlib>

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
#include <algorithm>
#include <unistd.h>


stringstream* GetBranchesStream(string fn, string trname, string branchestoberead) {

  stringstream* branchlist = new stringstream();
  string buffer;

  double bperc=atof(branchestoberead.c_str());

  if ( bperc != 0 ) {

    if ( bperc > 100 ) {
      cout << "Percentage to read > 100%" << endl;
      return 0;
    } else {
      cout << "generating branch list" << endl;
    }

    TFile *f;
    f=TFile::Open(fn.c_str());
    if (!f) {
      cout << "oops" << endl;
    }

    int filesize=f->GetSize();
    int sizecounter=0;
    int sizetarget=int((filesize*bperc/100));
    int branch=0;

    TTree *tree = (TTree*)f->Get(trname.c_str());
    TObjArray *a=tree->GetListOfBranches();
    int nentries = a->GetEntries();
    cout << "there are " << nentries << " branches" << endl;

    while (sizecounter < sizetarget) {
      //           cout << a->At(branch)->GetName() << "  " 
      //                << (reinterpret_cast<TBranch*>(a->At(branch)))->GetTotalSize() 
      //                << endl;
      sizecounter+=(reinterpret_cast<TBranch*>(a->At(branch)))->GetTotalSize();
      *branchlist << a->At(branch)->GetName() << endl;
      ++branch;
    }

    f->Close();
  } else {
    cout << "retrieving branch list from file" << endl;
    ifstream input(branchestoberead.c_str(), ios::in);
    if (! input) {
      cout << "Could not open " << branchestoberead.c_str() << endl;
      return 0;
    }
    if (input.is_open()){
      *branchlist<<input.rdbuf();
    }
    input.close();

  }
  return branchlist;

}
    

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

int read_OK(string fn, string trname, float percentage, float TTC, string branchesToBeRead = "", string proxyfn="", string certdir="", int endevent=-1, bool randomise=0){ 
    
    //gEnv->SetValue("TFile.AsyncPrefetching", 1);

    TStopwatch timer;
    Double_t nb = 0;	
    bool checkHDD=false;
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
    gEnv->SetValue("Davix.GSI.UserProxy", proxyfn.c_str());
    gEnv->SetValue("Davix.GSI.GridMode", "y");
    gEnv->SetValue("Davix.Debug", 0);

    // gEnv->SetValue("XNet.Debug", 1);
    // gEnv->SetValue("XNet.UseOldClient", "no");

    // Also setup the xrootd auth, in the case xrootd is used
    gEnv->SetValue("XSec.GSI.UserProxy", proxyfn.c_str());
   
    cout << "Opening '" << fn << "'" << endl; 
    f = TFile::Open(fn.c_str());
    if (!f) {
      cout << "Opening via TFile failed" << endl;
      f = new TNetFile(fn.c_str());
    } 

    if (!f) {
      cout << "File open failed... every hope is vain. Surrender.";
      return -1;
    }

    TTree *tree = (TTree*)f->Get(trname.c_str());
    TTreeCache::SetLearnEntries(1);

    Int_t nentries = (Int_t)tree->GetEntries();
    if (endevent > 0 ) nentries = endevent; 
    cout << "There are " << nentries << " events" << endl;

    tree->SetCacheSize((int)(TTC*1024*1024));

    if (branchesToBeRead.length()>0) {
      cout << "Retrieving a list of branches" << endl;
      tree->SetBranchStatus("*",0);
      stringstream *file;
      file=GetBranchesStream(fn, trname, branchesToBeRead);
      if (!file) {
        cout << "Error" << endl;
        exit(1);
      }
      while(!file->eof()) {
        string name="";
        *file>>name;
        if ( name.length() > 0 ) {
          cout << "Will use branch " << name.c_str() << endl;
          tree->SetBranchStatus(name.c_str(),1);
          tree->AddBranchToCache(name.c_str());
        }
      } 
      delete file;
    } else {
      cout << "Will use all branches" << endl;
      tree->AddBranchToCache("*",kTRUE);
    }

    tree->StopCacheLearningPhase();
    //TTreeCache::SetLearnEntries(1);
    TTreePerfStats *ps= new TTreePerfStats("ioperf",tree);

    vector<int> eventlist;
    for (int i=0;i<nentries;i++) {
      if (gRandom->Rndm(1)<(percentage/100)) {
        eventlist.push_back(i);
      }
    }

    cout << "Will read " << eventlist.size() << " events" << endl;
    if (randomise) {
      cout << "Randomising " << eventlist.size() << " events" << endl;
      // Stay repeatable
      srand(1234);
      random_shuffle(eventlist.begin(), eventlist.end());
    } else {
      cout << "No randomisation" << endl;
    }

    timer.Start();
    cout << "Starting event loop " << endl ;

    for (vector<int>::iterator iter=eventlist.begin(); iter!=eventlist.end(); ++iter) {
      // cout << "Reading " << *iter << endl;
      nb += tree->GetEntry(*iter);
    }

    timer.Stop();
  
    nb/=1024.0/1024.0;

    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    
    cout << endl;
    tree->PrintCacheStats();
    //ps->SaveAs("perf.root");
    cout << endl;
    ps->Print(); 

    cout<<endl;
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
	read_OK(argv[1],"physics",10,30,"0.17",getenv("X509_USER_PROXY"),"",-1,1);


}

