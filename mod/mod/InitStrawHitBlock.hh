///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __InitStrawHitBlock__
#define __InitStrawHitBlock__

#include <string.h>

#include "canvas/Utilities/InputTag.h"

#include "Stntuple/obj/TStnInitDataBlock.hh"
#include "Stntuple/obj/TStrawHitBlock.hh"

namespace stntuple {
class InitStrawHitBlock : public TStnInitDataBlock {
public:
  art::InputTag   fShCollTag;
  art::InputTag   fStrawDigiCollTag;
  art::InputTag   fStrawDigiMCCollTag;
  int             fWriteSdwf;
//-----------------------------------------------------------------------------
// functions
//-----------------------------------------------------------------------------
public:

  void   SetShCollTag          (art::InputTag& Tag) { fShCollTag          = Tag; }
  void   SetStrawDigiCollTag   (art::InputTag& Tag) { fStrawDigiCollTag   = Tag; }
  void   SetStrawDigiMCCollTag (art::InputTag& Tag) { fStrawDigiMCCollTag = Tag; }
  void   SetWriteSdwf          (int         Flag)   { fWriteSdwf          = Flag; }
  
  virtual int InitDataBlock  (TStnDataBlock* Block, AbsEvent* Evt, int Mode);
  //  virtual int ResolveLinks   (TStnDataBlock* Block, AbsEvent* Evt, int Mode);

};
}
#endif
