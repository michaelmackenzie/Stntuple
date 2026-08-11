///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __Stntuple_mod_InitTrackStrawHitBlock__
#define __Stntuple_mod_InitTrackStrawHitBlock__

#include <string.h>

#include "canvas/Utilities/InputTag.h"

#include "Stntuple/obj/TStnInitDataBlock.hh"
#include "Stntuple/obj/TTrackStrawHitBlock.hh"

namespace mu2e {
  class Tracker;
}

namespace stntuple {
class InitTrackStrawHitBlock : public TStnInitDataBlock {
public:
  art::InputTag         fKalRepPtrCollTag;
  art::InputTag         fKalSeedCollTag;
  art::InputTag         fShCollTag;
  art::InputTag         fStrawDigiCollTag;
  art::InputTag         fStrawDigiMCCollTag;

  const mu2e::Tracker*  tracker;
//-----------------------------------------------------------------------------
// functions
//-----------------------------------------------------------------------------
public:

  void   SetKalRepPtrCollTag   (std::string&   Tag) { fKalRepPtrCollTag   = art::InputTag(Tag); }
  void   SetKalSeedCollTag     (std::string&   Tag) { fKalSeedCollTag     = art::InputTag(Tag); }
  void   SetShCollTag          (art::InputTag& Tag) { fShCollTag          = Tag; }
  void   SetStrawDigiCollTag   (art::InputTag& Tag) { fStrawDigiCollTag   = Tag; }
  void   SetStrawDigiMCCollTag (art::InputTag& Tag) { fStrawDigiMCCollTag = Tag; }
  
  virtual int InitDataBlock  (TStnDataBlock* Block, AbsEvent* Evt, int Mode);
  //  virtual int ResolveLinks   (TStnDataBlock* Block, AbsEvent* Evt, int Mode);

};
}
#endif
