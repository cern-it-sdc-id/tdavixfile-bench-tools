#include "TFile.h"
#include "TTree.h"
#include "TStopwatch.h"
#include <iostream>

void inflateTree(const char *name = "h42",
                 const char *in = "root://eospps.cern.ch///eos/ppsscratch/test/h1big.root",
                 const char *out = "/tmp/h1big.root",  Int_t fact = 1)
{
	TStopwatch sw;
	sw.Start();
	
   // Get the input tree from the input file
   TFile *fin = TFile::Open(in);
   if (!fin || fin->IsZombie()) {
      Printf("inflateTree", "could not open input file: %s", in);
      return;
   }
   TTree *tin = (TTree *) fin->Get(name);
   if (!tin) {
      Printf("inflateTree", "could not find tree '%s' in %s", name, in);
      delete fin;
      return;
   }
   Long64_t nin = tin->GetEntriesFast();
   Printf("Input tree '%s' has %lld entries", name, nin);
   // Create output file
   TFile *fout = TFile::Open(out, "RECREATE", 0, 1);
   if (!fout || fout->IsZombie()) {
      Printf("inflateTree", "could not open input file: %s", in);
      delete fin;
      return;
   }
   // Clone the header of the initial tree
   TTree *tout= (TTree *) tin->CloneTree(0);
   tout->SetMaxTreeSize(19000000000);


   // Duplicate all entries once
#if 0
   Int_t nc = fact;
   while (nc--) {
      Printf("Writing copy %d ...", fact - nc);
      tout->CopyEntries(tin);
   }
#else
   for (Long64_t i = 0; i < nin; ++i) {
      if (tin->LoadTree(i) < 0) {
         break;
      }
      tin->GetEntry(i);
      Int_t nc = fact;
      while (nc--) {
         tout->Fill();
      }
      if (i > 0 && !(i%1000)) {
         Printf("%d copies of %lld entries filled ...", fact, i);
      }
   }
#endif
   // Finalize the writing out
   tout->Write();
   
   // print perf stats
   

    sw.Stop();
    std::cout << "Drawing. Realtime: " <<      sw.RealTime()  << std::endl;
    std::cout << "Drawing. Cputime : " <<      sw.CpuTime()  << std::endl;
	tin->PrintCacheStats();   
   
   
   // Close the files
   fout->Close();
   fin->Close();
   // Cleanup
   delete fout;
   delete fin;
}


int main(int argc, char** argv){
//gDebug=0x02;

inflateTree("h42", argv[1]);


return 0;

}


