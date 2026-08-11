#include <iostream>
#include <iomanip>

#include "obj/TStnTimeClusterBlock.hh"
#include "obj/TStnTimeCluster.hh"

ClassImp(TStnTimeClusterBlock)

//_____________________________________________________________________________
void TStnTimeClusterBlock::Streamer(TBuffer &R__b) {
  // Stream an object of class TStnTrackBlock.

  if (R__b.IsReading()) {
    Version_t R__v = R__b.ReadVersion(); if (R__v) { }
    R__b >> fNTimeClusters;
    fListOfTimeClusters->Streamer(R__b);
    // for (int i=0; i<fNTracks; i++) {
    //   Track(i)->SetNumber(i);
    // }
  } 
  else {
    R__b.WriteVersion(TStnTimeClusterBlock::IsA());
    R__b << fNTimeClusters;
    fListOfTimeClusters->Streamer(R__b);
  }
}
TStnTimeClusterBlock::TStnTimeClusterBlock() {
  fNTimeClusters      = 0;
  fListOfTimeClusters = new TClonesArray("TStnTimeCluster",100);
  //  fListOfTimeClusters->BypassStreamer(kFALSE);
  fListOfTimeClusters->BypassStreamer(kTRUE);
  fCollName  = "default";
}


//_____________________________________________________________________________
TStnTimeClusterBlock::~TStnTimeClusterBlock() {
  fListOfTimeClusters->Delete();
  delete fListOfTimeClusters;
}


//_____________________________________________________________________________
void TStnTimeClusterBlock::Clear(Option_t* opt) {
  fNTimeClusters = 0;
  fListOfTimeClusters->Clear(opt);

  f_EventNumber       = -1;
  f_RunNumber         = -1;
  f_SubrunNumber      = -1;
  fLinksInitialized   =  0;
}

//------------------------------------------------------------------------------
void TStnTimeClusterBlock::Print(Option_t* opt) const {

  int banner_printed = 0;
  for (int i=0; i<fNTimeClusters; i++) {
    TStnTimeCluster* t = ((TStnTimeClusterBlock*) this)->TimeCluster(i);
    if (! banner_printed) {
      t->Print("banner");
      banner_printed = 1;
    }
    t->Print("data");
  }
}
