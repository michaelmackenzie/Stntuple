//
// Read the tracks added to the event by the track finding and fitting code.
//
// $Id: TStnCluster.hh,v 1.1 2014/06/13 06:14:48 murat Exp $
// $Author: murat $
// $Date: 2014/06/13 06:14:48 $
//
// Contact person Pavel Murat
//
#ifndef murat_inc_TStnCluster_hh
#define murat_inc_TStnCluster_hh

// storable objects (data products)
//

// C++ includes.
#include <iostream>

#include "TString.h"
#include "TFolder.h"
#include "TFile.h"
#include "TBuffer.h"

//namespace murat {

class TStnTrack;

namespace mu2e {
  class CaloCluster;
}

class TStnCluster : public TObject {

  enum {
    kNFreeIntsV1   = 10,		// V1
    kNFreeFloatsV1 = 10,		// V1
    kNFreeIntsV2   = 10, // V2
    kNFreeFloatsV2 =  3, // V2

    kNFreeInts     =  8,		// V3
    kNFreeFloats   =  1,		// V3

    kNFreeFloats2  = 10 // more space for expansion added in V3
  };
  enum { kMaxCrystals = 100 };

public:
//-----------------------------------------------------------------------------
// integers
//-----------------------------------------------------------------------------
  int                       fNumber;          // index in the list of reconstructed clusters
  int                       fDiskID;	      //
  int                       fNCrystals;       //
  int                       fNCr1     ;       // N crystals above 1 MeV
  int                       fTrackNumber;     // closest track in TStnTrackBlock
  int                       fIx1;	      // [row, column] or [x1,x2] for a disk -- Not used
  int                       fIx2;
  int                       fMCSimID;         // Sim particle ID with the highest energy deposit, added in V3
  int                       fMCSimPDG;        // Sim particle PDG with the highest energy deposit, added in V3
  int                       fInt[kNFreeInts];
//-----------------------------------------------------------------------------
// floats
//-----------------------------------------------------------------------------
  float                     fX;
  float                     fY;
  float                     fZ;
  float                     fYMean;
  float                     fZMean;     // Currently actually X mean
  float                     fSigY;      // cluster width in Y
  float                     fSigZ;	// cluster width in Z/X (fSigZ is reused as sig X)
  float                     fSigR;      // don't know what it is
  float                     fEnergy   ;
  float                     fTime     ;
  float                     fFrE1     ; // e1/etotal
  float                     fFrE2     ; // (e1+e2)/etotal
  float                     fSigE1    ;
  float                     fSigE2    ;
//-----------------------------------------------------------------------------
// 7 words added in version 2
//-----------------------------------------------------------------------------
  float                     fSigXX;      // sums over crystals
  float                     fSigXY;
  float                     fSigYY;
  float                     fNx   ;      // cluster direction, cos(phi)
  float                     fNy   ;      // sin(phi)
//-----------------------------------------------------------------------------
// 9 words + expansion space added in version 3
//-----------------------------------------------------------------------------
  float                     fTimeRMS;    // RMS of hit times within the cluster
  float                     fMaxR ;      // max crystal distance from most energetic crystal
  float                     fFloat[kNFreeFloats];

  float                     fE9;         // Energy of main crystal + neighbors
  float                     fE25;        // Energy of main crystal + neighbors and their neighbors
  float                     fOutRingE;   // Energy of cluster crystals with R > 600 mm
  float                     fMCSimEDep;  // MC main sim energy deposit, added in V3
  float                     fMCSimMomIn; // MC main sim momentum in, added in V3
  float                     fMCEDep;     // MC energy deposited, added in V3
  float                     fMCTime;     // MC time, weighted by sim energy dep, added in V3
  float                     fFloat2[kNFreeFloats2]; // Extra expansion room added in V3
//-----------------------------------------------------------------------------
// sparse crystal hit information
//-----------------------------------------------------------------------------
  float                     fCrystalEnergies[kMaxCrystals];
  float                     fCrystalTimes   [kMaxCrystals];
  int                       fCrystalIDs     [kMaxCrystals];
//-----------------------------------------------------------------------------
// transients
//-----------------------------------------------------------------------------
  const mu2e::CaloCluster*  fCaloCluster;  //!
  TStnTrack*                fClosestTrack; //!
//-----------------------------------------------------------------------------
// methods
//-----------------------------------------------------------------------------
  TStnCluster(int i = -1);
  ~TStnCluster();
//-----------------------------------------------------------------------------
// accessors
//-----------------------------------------------------------------------------
  int     DiskID      () const { return fDiskID  ; }
  int     Ix1         () const { return fIx1; }
  int     Ix2         () const { return fIx2; }
  int     NCrystals   () const { return fNCrystals; }
  int     Number      () const { return fNumber; }
  int     TrackNumber () const { return fTrackNumber; }

  float   Energy      () const { return fEnergy; }
  float   Time        () const { return fTime  ; }

  float   Nx          () const { return fNx;    }
  float   Ny          () const { return fNy;    }

  float   SeedFr      () const { return fFrE1;  }
  float   SeedFr2     () const { return fFrE2;  }

  float   SigX        () const { return fSigZ;  }
  float   SigY        () const { return fSigY;  }
  float   SigZ        () const { return fSigZ;  }

  float   SigXX       () const { return fSigXX; }
  float   SigXY       () const { return fSigXY; }
  float   SigYY       () const { return fSigYY; }

  float   TimeRMS     () const { return fTimeRMS;}
  float   MaxDeltaR   () const { return fMaxR;  }
  float   E9          () const { return fE9;    }
  float   E25         () const { return fE25;   }
  float   OutRingE    () const { return fOutRingE;}

  // evaluated values
  float   E1          () const { return SeedFr() * Energy(); } // energy of main hit
  float   E2          () const { return SeedFr2() * Energy() - E1(); } // energy of the second biggest hit
  float   E12         () const { return SeedFr2() * Energy(); } // energy of main two hits
  float   RingE       () const { return E9() - E1(); } // energy around the main hit
  float   R           () const { return std::sqrt(fX*fX + fY*fY); }

  // sparse crystal info
  float   CrystalE    (const int index) const {
    (index > 0 && index < NCrystals()) ? fCrystalEnergies[index] : 0.f;
  }
  float   CrystalT    (const int index) const {
    (index > 0 && index < NCrystals()) ? fCrystalTimes[index] : 0.f;
  }
  int   CrystalID   (const int index) const {
    (index > 0 && index < NCrystals()) ? fCrystalIDs[index] : 0;
  }

  // linked track
  TStnTrack* ClosestTrack() { return fClosestTrack; }

  void    SetNumber(int I) { fNumber = I; } //

//-----------------------------------------------------------------------------
// overloaded methods of TObject
//-----------------------------------------------------------------------------
  virtual void Clear(Option_t* Opt = "") ;
  virtual void Print(Option_t* Opt = "") const ;
//-----------------------------------------------------------------------------
// schema evolution
//-----------------------------------------------------------------------------
  void ReadV1(TBuffer& R__b);

  ClassDef(TStnCluster,4)
};

#endif
