///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __InitCalDigiBlock__
#define __InitCalDigiBlock__

#include <string.h>

#include "canvas/Utilities/InputTag.h"

#include "Stntuple/obj/TStnInitDataBlock.hh"
#include "Stntuple/obj/TCalDigiBlock.hh"

class StntupleInitCalDigiBlock : public TStnInitDataBlock {
public:
  art::InputTag   fCalDigiCollTag;
//-----------------------------------------------------------------------------
// functions
//-----------------------------------------------------------------------------
public:

  void   SetCalDigiCollTag (art::InputTag& Tag) { fCalDigiCollTag = Tag; }
  //   void   SetStrawDigiMCCollTag (art::InputTag& Tag) { fStrawDigiMCCollTag = Tag; }
  
  virtual int InitDataBlock    (TStnDataBlock* Block, AbsEvent* Evt, int Mode);
  virtual int ResolveLinks     (TStnDataBlock* Block, AbsEvent* Evt, int Mode);

};

#endif
