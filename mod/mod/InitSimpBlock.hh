///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __STNTUPLE_InitSimpBlock__
#define __STNTUPLE_InitSimpBlock__

#include <string.h>

#include "canvas/Utilities/InputTag.h"

#include "Stntuple/obj/TStnInitDataBlock.hh"
#include "Stntuple/obj/TSimpBlock.hh"

class StntupleInitSimpBlock : public TStnInitDataBlock {
public:
  art::InputTag   fSimpCollTag;
  art::InputTag   fSpmcCollTag;        // need for proper time
  art::InputTag   fShCollTag;
  art::InputTag   fStrawDigiMCCollTag;
  art::InputTag   fVDHitsCollTag;
  art::InputTag   fPrimaryParticleTag;
  art::InputTag   fCaloClusterMCCollTag; // for accepting sims with calo energy deposit
  float           fMinSimpMomentum;
  float           fMaxZ;
  int             fGenProcessID;
  int             fPdgID;
  int             fMinNStrawHits;
//-----------------------------------------------------------------------------
// functions
//-----------------------------------------------------------------------------
public:

  void   SetSimpCollTag          (art::InputTag& Tag ) { fSimpCollTag           = Tag ; }
  void   SetShCollTag            (art::InputTag& Tag ) { fShCollTag             = Tag ; }
  void   SetSdmcCollTag          (art::InputTag& Tag ) { fStrawDigiMCCollTag    = Tag ; }
  void   SetSpmcCollTag          (art::InputTag& Tag ) { fSpmcCollTag           = Tag ; }
  void   SetVDHitsCollTag        (art::InputTag& Tag ) { fVDHitsCollTag         = Tag ; }
  void   SetPrimaryParticleTag   (art::InputTag& Tag ) { fPrimaryParticleTag    = Tag ; }
  void   SetCaloClusterMCCollTag (art::InputTag& Tag ) { fCaloClusterMCCollTag  = Tag ; }
  void   SetMinSimpMomentum   (double         MinP) { fMinSimpMomentum    = MinP; }
  void   SetMaxZ              (double         MaxZ) { fMaxZ               = MaxZ; }
  void   SetGenProcessID      (int            ID  ) { fGenProcessID       = ID  ; }
  void   SetPdgID             (int            ID  ) { fPdgID              = ID  ; }
  void   SetMinNStrawHits     (int            N   ) { fMinNStrawHits      = N   ; }

  virtual int InitDataBlock(TStnDataBlock* Block, AbsEvent* Evt, int Mode);
  //  virtual int ResolveLinks (TStnDataBlock* Block, AbsEvent* Evt, int Mode);

};

#endif
