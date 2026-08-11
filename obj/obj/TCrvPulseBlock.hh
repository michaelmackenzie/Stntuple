#ifndef STNTUPLE_TCrvPulseBlock
#define STNTUPLE_TCrvPulseBlock

#include "TClonesArray.h"

#include "Stntuple/obj/TStnLinkBlock.hh"
#include "Stntuple/obj/TStnDataBlock.hh"
#include "Stntuple/obj/TCrvRecoPulse.hh"

#include "Stntuple/mod/InitStntupleDataBlocks.hh"

class TCrvPulseBlock: public TStnDataBlock {
  friend class StntupleInitCrvPulseBlock;

public:
  int            fNPulses;	             // number of reconstructed pulses
  TClonesArray*  fListOfPulses;	             // list of pulses
//-----------------------------------------------------------------------------
//  functions
//-----------------------------------------------------------------------------
public:
					// ****** constructors and destructor
  TCrvPulseBlock();
  virtual ~TCrvPulseBlock();
					// ****** accessors

  Int_t                   NPulses         () { return fNPulses; }
  TCrvRecoPulse*          Pulse      (int i) { return (TCrvRecoPulse*) fListOfPulses->UncheckedAt(i); }
  TClonesArray*           GetListOfPulses () { return fListOfPulses; }
//-----------------------------------------------------------------------------
// modifiers
//-----------------------------------------------------------------------------
  TCrvRecoPulse*          NewPulse() { 
    return new ((*fListOfPulses)[fNPulses++]) TCrvRecoPulse();
  }
//-----------------------------------------------------------------------------
// overloaded methods of TObject
//-----------------------------------------------------------------------------
  void Clear(Option_t* opt="");
  void Print(Option_t* opt="") const;

  ClassDef(TCrvPulseBlock,1)	// CRV reco block
};


#endif
