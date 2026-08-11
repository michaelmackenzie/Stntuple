//
// 2014-01-27 P.Murat
//

#include "Stntuple/obj/TCrvPulseBlock.hh"

ClassImp(TCrvPulseBlock)
//-----------------------------------------------------------------------------
// R_v is, so far, unused
//-----------------------------------------------------------------------------
  void TCrvPulseBlock::Streamer(TBuffer &R__b) {
  if(R__b.IsReading()) {
    //    Version_t R__v = R__b.ReadVersion();
    R__b.ReadVersion();
    R__b >> fNPulses;
    fListOfPulses->Streamer(R__b);

    // R__b >> fNCoincidences;
    // fListOfCoincidences->Streamer(R__b);
    // fCoincidencePulseLinks->Streamer(R__b);
  }
  else {
    R__b.WriteVersion(TCrvPulseBlock::IsA());
    R__b << fNPulses;
    fListOfPulses->Streamer(R__b);

    // R__b << fNCoincidences;
    // fListOfCoincidences->Streamer(R__b);
    // fCoincidencePulseLinks->Streamer(R__b);
  }
}

//-----------------------------------------------------------------------------
TCrvPulseBlock::TCrvPulseBlock() {
  fListOfPulses = new TClonesArray("TCrvRecoPulse",1000);
  fListOfPulses->BypassStreamer(kFALSE);

  // fListOfCoincidences = new TClonesArray("TCrvCoincidence",100);
  // fListOfCoincidences->BypassStreamer(kFALSE);

  //   fCoincidencePulseLinks = new TStnLinkBlock();

  Clear();
}

//______________________________________________________________________________
TCrvPulseBlock::~TCrvPulseBlock() {
  fListOfPulses->Delete();
  delete fListOfPulses;

  // fListOfCoincidences->Delete();
  // delete fListOfCoincidences;

  // delete fCoincidencePulseLinks;
}

//______________________________________________________________________________
void TCrvPulseBlock::Clear(Option_t* opt) {
  fNPulses = 0;
  fListOfPulses->Clear();

  //  fNCoincidences = 0;
  // fListOfCoincidences->Clear();
  // fCoincidencePulseLinks->Clear();

  f_EventNumber       = -1;
  f_RunNumber         = -1;
  f_SubrunNumber      = -1;
  fLinksInitialized   =  0;
}

//______________________________________________________________________________
void TCrvPulseBlock::Print(Option_t* opt) const {
  // print all hits in the straw tracker
  printf(" *** reconstructed CRV pulses *** \nNumber: %d\n",fNPulses);
  int banner_printed = 0;
  TCrvPulseBlock* block = (TCrvPulseBlock*) this; // bypassing constness...

  for(int i=0; i<fNPulses; i++) {
    TCrvRecoPulse* p = block->Pulse(i);
    if (! banner_printed) {
      p->Print("banner");
      banner_printed = 1;
    }
    p->Print("data");
  }

  // printf(" *** reconstructed CRV coincidences *** \nNumber: %d\n",fNCoincidences);
  // for(int i=0; i<fNCoincidences; i++) {
  //   fListOfCoincidences->At(i)->Print();
  // }
}

