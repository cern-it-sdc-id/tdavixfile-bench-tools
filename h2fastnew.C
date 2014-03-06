   void h2fast(const char *url , Bool_t draw=kFALSE, Long64_t cachesize=10000000, Int_t learn=1) {
gEnv->SetValue("TFile.AsyncReading", "no");
      gEnv->SetValue("TFile.ForceRemote", "yes");

   TStopwatch sw;
   sw.Start();
   Long64_t oldb = TFile::GetFileBytesRead();
   TFile *f = TFile::Open(url);
  
   if (!f || f->IsZombie()) {
      printf("File h1big.root does not exist\n");
      return;
   }

   TTreeCacheUnzip::SetParallelUnzip(TTreeCacheUnzip::kEnable);

   T= (TTree*)f->Get("h42");
   Long64_t nentries = T->GetEntries();
   T->SetCacheSize(cachesize);
   TTreeCache::SetLearnEntries(learn);
   TTreeCache *tpf = f->GetCacheRead();
   //tpf->SetEntryRange(0,nentries);
   
   if (draw) T->Draw("rawtr","E33>20");
   else {
      TBranch *brawtr = T->GetBranch("rawtr");
      TBranch *bE33   = T->GetBranch("E33");
      Float_t E33; 
      bE33->SetAddress(&E33);
      for (Long64_t i=0;i<nentries;i++) {
         T->LoadTree(i);
         bE33->GetEntry(i);
         if (E33 > 0) brawtr->GetEntry(i);
      } 
   } 
   if (tpf) tpf->Print();
   printf("Bytes read = %lld\n",TFile::GetFileBytesRead()-oldb);
   printf("Real Time = %7.3f s, CPUtime = %7.3f s\n",sw.RealTime(),sw.CpuTime());
   //delete T;
   //delete f;
}
