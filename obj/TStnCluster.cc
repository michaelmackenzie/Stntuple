//
#include <iostream>

#include "TMatrix.h"
#include "obj/TStnCluster.hh"

// #include "RecoDataProducts/inc/CaloCluster.hh"

namespace {
  //  const double BF = 1.4116 ;  // CDF case
  const double BF = 1.0 ;         // MU2E case
}

//namespace murat {

ClassImp(TStnCluster)

//-----------------------------------------------------------------------------
// Read an object of class TStnCluster (version 1).
// V1 didn't store the cluster asymmetry data
//-----------------------------------------------------------------------------
void TStnCluster::ReadV1(TBuffer &R__b) {

  struct TStnClusterDataV1_t {
    int                       fNumber;             // index in the list of reconstructed clusters
    int                       fDiskID;	           // 
    int                       fNCrystals;          //
    int                       fNCr1     ;          // above 1 MeV
    int                       fTrackNumber;        // closest track in TStnTrackBlock
    int                       fIx1;	           // [row, column] or [x1,x2] for a disk
    int                       fIx2;
    int                       fInt[kNFreeIntsV1];
					           // floats
    float                     fX;
    float                     fY;
    float                     fZ;
    float                     fYMean;
    float                     fZMean;
    float                     fSigY;
    float                     fSigZ;
    float                     fSigR;
    float                     fEnergy;
    float                     fTime     ; 
    float                     fFrE1     ; // e1/etotal
    float                     fFrE2     ; // (e1+e2)/etotal
    float                     fSigE1    ;
    float                     fSigE2    ;
    float                     fFloat[kNFreeFloatsV1];
  };

  TStnClusterDataV1_t data;
  
  int                 nwi, nwf;
  
  nwi = (int*  ) data.fInt   - &data.fNumber + kNFreeIntsV1;
  nwf = (float*) data.fFloat - &data.fX      + kNFreeFloatsV1;
  
  R__b.ReadFastArray(&data.fNumber,nwi);
  R__b.ReadFastArray(&data.fX     ,nwf);
  
  fNumber        = data.fNumber      ;          // track index in the list of reconstructed clusters
  fDiskID        = data.fDiskID      ;	      // 
  fNCrystals     = data.fNCrystals   ;       //
  fNCr1          = data.fNCr1        ;       // above 1 MeV
  fTrackNumber   = data.fTrackNumber ;     // closest track in TStnTrackBlock
  fIx1           = data.fIx1         ;	      // [row, column] or [x1,x2] for a disk
  fIx2           = data.fIx2         ;
		  	       	          	// float part
  fX             = data.fX           ;
  fY             = data.fY           ;
  fZ             = data.fZ           ;
  fYMean         = data.fYMean       ;
  fZMean         = data.fZMean       ;
  fSigY          = data.fSigY        ; 
  fSigZ          = data.fSigZ        ;
  fSigR          = data.fSigR        ;
  fEnergy        = data.fEnergy      ;
  fTime          = data.fTime        ; 
  fFrE1          = data.fFrE1        ; // e1/etotal
  fFrE2          = data.fFrE2        ; // (e1+e2)/etotal
  fSigE1         = data.fSigE1       ;
  fSigE2         = data.fSigE2       ;
//-----------------------------------------------------------------------------
// initialize the V2 part. make sure teh numbers don't make sense
//-----------------------------------------------------------------------------
  fSigXX         = -1.;     // sums over crystals
  fSigXY         = -1.;
  fSigYY         = -1.;
  fNx            =  1.;     // cluster direction
  fNy            =  0.;
  fTimeRMS       =  0.;     // added in V3
  fMaxR          =  0.;     // added in V3
  fE9            =  0.;     // added in V3
  fE25           =  0.;     // added in V3
  fOutRingE      =  0.;     // added in V3
  fMCSimID       =  -1;     // added in V3
  fMCSimPDG      =  0 ;     // added in V3
  fMCSimEDep     = -1.;     // added in V3
  fMCSimMomIn    = -1.;     // added in V3
  fMCEDep        = -1.;     // added in V3
  fMCTime        =  0.;     // added in V3
}

//-----------------------------------------------------------------------------
void TStnCluster::Streamer(TBuffer& R__b) {
  int nwi, nwf, nwf2;

  nwi = (int*  ) &fInt   - &fNumber + kNFreeInts;
  nwf = (float*) &fFloat - &fX      + kNFreeFloats ;
  nwf2= (float*) &fFloat2- &fMCTime + kNFreeFloats2;

  if (R__b.IsReading()) {
    Version_t R__v = R__b.ReadVersion(); 

    if (R__v == 1) ReadV1(R__b);
    else {
					// current version: V3
      R__b.ReadFastArray(&fNumber,nwi);
      R__b.ReadFastArray(&fX     ,nwf);
      if(R__v < 3) { // added in version 3
        fTimeRMS       =  0.;
        fMaxR          =  0.;
        fE9            =  0.;
        fE25           =  0.;
        fOutRingE      =  0.;
        fMCSimID       = -1 ;
        fMCSimPDG      =  0 ;
        fMCSimEDep     = -1.;
        fMCSimMomIn    = -1.;
        fMCEDep        = -1.;
        fMCTime        =  0.;
      } else {
        R__b.ReadFastArray(&fE9,nwf2); // Extra float region added in V3
      }
      const int ncr = NCrystals();
      if(R__v < 4) { // sparse crystal info added in V4
        for(int index = 0; index < ncr; ++index) {
          fCrystalEnergies[index] = 0.f;
          fCrystalTimes   [index] = 0.f;
          fCrystalIDs     [index] = 0  ;
        }
      } else {
        R__b.ReadFastArray(fCrystalEnergies,ncr);
        R__b.ReadFastArray(fCrystalTimes   ,ncr);
        R__b.ReadFastArray(fCrystalIDs     ,ncr);
      }
    }
  }
  else {
    R__b.WriteVersion(TStnCluster::IsA());

    R__b.WriteFastArray(&fNumber,nwi );
    R__b.WriteFastArray(&fX     ,nwf );
    R__b.WriteFastArray(&fE9    ,nwf2);
    const int ncr = NCrystals();
    R__b.WriteFastArray(fCrystalEnergies,ncr);
    R__b.WriteFastArray(fCrystalTimes   ,ncr);
    R__b.WriteFastArray(fCrystalIDs     ,ncr);
  }
}

//_____________________________________________________________________________
TStnCluster::TStnCluster(Int_t Number) {
  // 'Number' can be -1 ...
  
  fNumber       = Number;
  fNCrystals    = 0;
  fCaloCluster  = 0;
  fClosestTrack = 0;
  fTimeRMS      =  0.f;
  fMaxR         =  0.f;
  fE9           =  0.f;
  fE25          =  0.f;
  fOutRingE     =  0.f;
  fMCSimID      = -1;
  fMCSimPDG     =  0;
  fMCSimEDep    = -1.f;
  fMCSimMomIn   = -1.f;
  fMCEDep       = -1.f;
  fMCTime       =  0.f;
}


//_____________________________________________________________________________
TStnCluster::~TStnCluster() {
}


//-----------------------------------------------------------------------------
void TStnCluster::Clear(Option_t* opt) {
  Error("Print", "Not implemented yet");
  fTimeRMS      =  0.f;
  fMaxR         =  0.f;
  fE9           =  0.f;
  fE25          =  0.f;
  fOutRingE     =  0.f;
  fMCSimID      = -1;
  fMCSimPDG     =  0;
  fMCSimEDep    = -1.f;
  fMCEDep       = -1.f;
  fMCTime       =  0.f;
}

//-----------------------------------------------------------------------------
void TStnCluster::Print(Option_t* Option) const {

  TString opt(Option);

  opt.ToLower();
					// "non-const *this" for printing purposes
  TStnCluster* cl = (TStnCluster*) this;

  if ((opt == "") || (opt.Index("banner") >= 0)) {
//-----------------------------------------------------------------------------
// print banner
//-----------------------------------------------------------------------------
    printf("-------------------------------------------------------\n");
    printf("  i  disk ncr energy    time    X    Y    Z    nX    nY\n");
    printf("-------------------------------------------------------\n");
  }

  if ((opt == "") || (opt.Index("data") >= 0)) {
    printf("%4i %3i %3i %8.3f %8.3f ",
	   fNumber,cl->DiskID(), cl->fNCrystals, cl->fEnergy, cl->fTime);
    printf(" %8.3f %8.3f %8.3f %8.3f %8.3f",
	   cl->fX, cl->fY, cl->fZ, cl->fNx, cl->fNy);
    printf("\n");
  }
}

// } // end namespace




