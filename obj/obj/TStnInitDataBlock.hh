#ifndef STNTUPLE_TStnInitDataBlock
#define STNTUPLE_TStnInitDataBlock
//-----------------------------------------------------------------------------
//  base class for initialization of STNTUPLE data block
//  Author:    P.Murat (Mu2e/FNAL)
//  Date:      Feb 10 2019
//-----------------------------------------------------------------------------
#include "Stntuple/obj/AbsEvent.hh"
#include "TObject.h"

// class TStnEvent;
class TStnDataBlock;

class TStnInitDataBlock: public TObject {
public:
//-----------------------------------------------------------------------------
//  constructors and destructor
//-----------------------------------------------------------------------------
  TStnInitDataBlock();
  virtual ~TStnInitDataBlock();
//-----------------------------------------------------------------------------
//  data block initialization functions
//-----------------------------------------------------------------------------
  virtual int InitDataBlock(TStnDataBlock*, AbsEvent*, int Mode); 
  virtual int ResolveLinks (TStnDataBlock*, AbsEvent*, int Mode);
  virtual int BeginRun     (int RunNumber);
};
#endif
