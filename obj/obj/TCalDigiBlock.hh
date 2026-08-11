#ifndef TCalDigiBlock_hh
#define TCalDigiBlock_hh

#include "TClonesArray.h"

#include "Stntuple/obj/TStnDataBlock.hh"
#include "Stntuple/mod/InitStntupleDataBlocks.hh"
#include "Stntuple/obj/TCalDigi.hh"
#include "TBuffer.h"

class TCalDigiBlock : public TStnDataBlock {
  friend Int_t StntupleInitMu2eCalDigiBlock(TStnDataBlock*, AbsEvent*, int);
public:
                                        // this is version v1
  int            fNDigis;		//
  TClonesArray*  fListOfCalDigis;	//
//-----------------------------------------------------------------------------
//  functions
//-----------------------------------------------------------------------------
public:

  TCalDigiBlock();
  virtual ~TCalDigiBlock();
					// ****** accessors

  Int_t         NDigis         () { return fNDigis; }

  TCalDigi*  CalDigi(int I) { 
    return (TCalDigi*) fListOfCalDigis->UncheckedAt(I);
  }

  TClonesArray* GetListOfCalDigis () { return fListOfCalDigis ; } 
//-----------------------------------------------------------------------------
// modifiers
//-----------------------------------------------------------------------------
  TCalDigi*  NewCalDigi() { 
    return new ((*fListOfCalDigis)[fNDigis++]) TCalDigi();
  }
//-----------------------------------------------------------------------------
// schema evolution
//-----------------------------------------------------------------------------
  void        ReadV1(TBuffer& R__b);
//-----------------------------------------------------------------------------
// overloaded methods of TObject
//-----------------------------------------------------------------------------
  void        Clear(Option_t* opt="") override;
  void        Print(Option_t* opt="") const override;

  ClassDefOverride(TCalDigiBlock,1)
};

#endif
