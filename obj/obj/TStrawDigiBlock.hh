#ifndef TStrawDigiBlock_hh
#define TStrawDigiBlock_hh

#include "TClonesArray.h"

#include "Stntuple/obj/TStnDataBlock.hh"
#include "Stntuple/mod/InitStntupleDataBlocks.hh"
#include "Stntuple/obj/TStrawDigi.hh"
#include "TBuffer.h"

class TStrawDigiBlock : public TStnDataBlock {
  friend Int_t StntupleInitMu2eStrawDigiBlock(TStnDataBlock*, AbsEvent*, int);
public:
                                        // this is version v1
  int            fNDigis;		//
  int            fEwTag1024[36];        // EW Tag % 1024
  TClonesArray*  fListOfDigis;          //
  int            fNAdcSamples;          //! transient
//-----------------------------------------------------------------------------
//  functions
//-----------------------------------------------------------------------------
public:

  TStrawDigiBlock();
  virtual ~TStrawDigiBlock();
					// ****** accessors

  Int_t         NDigis         () { return fNDigis; }

  int           EwTag1024(int I) {
    return fEwTag1024[I];
  }

  TStrawDigi*  StrawDigi(int I) { 
    return (TStrawDigi*) fListOfDigis->UncheckedAt(I);
  }

  TClonesArray* GetListOfDigis () { return fListOfDigis ; } 
//-----------------------------------------------------------------------------
// modifiers
//-----------------------------------------------------------------------------
  TStrawDigi*  NewDigi() { 
    return new ((*fListOfDigis)[fNDigis++]) TStrawDigi();
  }
//-----------------------------------------------------------------------------
// schema evolution
//-----------------------------------------------------------------------------
  void        ReadV1(TBuffer& R__b);
//-----------------------------------------------------------------------------
// overloaded methods of TObject
//-----------------------------------------------------------------------------
  void        Clear(Option_t* opt="");
  void        Print(Option_t* opt="") const;

  ClassDefOverride(TStrawDigiBlock,1)
};

#endif
