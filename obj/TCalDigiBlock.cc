///////////////////////////////////////////////////////////////////////////////
//  Dec 07 2001 P.Murat: start putting in some comments
//  ---------------------------------------------------
// TCalDigiBlock: ROOT-parseable description of TCalData to be stored in 
//                STNTUPLE
///////////////////////////////////////////////////////////////////////////////
#include <format>
#include "TVector2.h"

#include "Stntuple/obj/TCalDigiBlock.hh"

ClassImp(TCalDigiBlock)

// //_____________________________________________________________________________
// void TCalDigiBlock::ReadV1(TBuffer &R__b) {

//   struct TCalDigiBlockV1_t {
//     int            fNHits;		// number of hit crystals
//     int            fNDisks;             // 
//     int            fNCrystals  [4];	// 
//     float          fRMin       [4];	// as a temporary measure, store 
//     float          fRMax       [4];
//     float          fZ0         [4];     // 
//     float          fCrystalSize;
//     float          fMinFraction;        // min fr of the included crystal area

//     TClonesArray*  fListOfCalDigis;	// list of crystal hit data 
//   };

//   TCalDigiBlockV1_t data; 

//   int nwi = ((int*  ) data.fRMin       ) - &data.fNHits;
//   int nwf = ((float*) &data.fListOfCalDigis) - data.fRMin;

//   R__b.ReadFastArray(&fNHits,nwi);
//   R__b.ReadFastArray(fRMin  ,nwf);

//   if (fNHits > 0) {
//     fListOfCalDigis->Streamer(R__b);
//   }
// 				// initialize V2 variables 
//   fWrapperThickness = 0.065;    // 65 microns
//   fShellThickness   = 0;
// }



//______________________________________________________________________________
void TCalDigiBlock::Streamer(TBuffer &R__b) {
  // Stream an object of class TCalDigiBlock.

  // int nwi = ((int*  ) &fListOfCalDigis) - &fNDigis;
  // int nwf = 0; // ((float*) &fListOfCalDigis) - fRMin;

  if (R__b.IsReading()) {
    // Version_t R__v = R__b.ReadVersion(); 
    // if      (R__v == 1) ReadV1(R__b);
    // else if (R__v == 2) ReadV2(R__b);
    R__b >> fNDigis;
    if (fNDigis > 0) {
      fListOfCalDigis->Streamer(R__b);
    }
//     else {
// //-----------------------------------------------------------------------------
// // read version > 1 ???
// //-----------------------------------------------------------------------------
//       std::cout << std::format(">>> WARNING: TCalDigiBlock::Streamer read version:{} fNDigis:{}\n",R__v,fNDigis);
//    } 
  }
  else {
    R__b.WriteVersion(TCalDigiBlock::IsA());
    R__b << fNDigis;

    if (fNDigis > 0) {
      fListOfCalDigis->Streamer(R__b);
    }
  }
}

//_____________________________________________________________________________
TCalDigiBlock::TCalDigiBlock() {

  fListOfCalDigis    = new TClonesArray("TCalDigi",100);
  fListOfCalDigis->BypassStreamer(kFALSE);
  Clear();
}

//_____________________________________________________________________________
TCalDigiBlock::~TCalDigiBlock() {
  fListOfCalDigis->Delete();
  delete fListOfCalDigis;
}

//_____________________________________________________________________________
void TCalDigiBlock::Clear(Option_t* opt) {
  fListOfCalDigis->Clear();
  fNDigis             = 0;

  f_EventNumber       = -1;
  f_RunNumber         = -1;
  f_SubrunNumber      = -1;
  fLinksInitialized   =  0;
}

//_____________________________________________________________________________
void TCalDigiBlock::Print(Option_t* opt) const {
  // print all the towers in the list
  if (fNDigis > 0) {
    fListOfCalDigis->At(0)->Print("banner");
    for (int i=0; i<fNDigis; i++) {
      fListOfCalDigis->At(i)->Print();
    }
  }
}
