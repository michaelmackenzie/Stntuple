///////////////////////////////////////////////////////////////////////////////
// 2019-06-10 P.Murat: base class for STNTUPLE data block initialization
///////////////////////////////////////////////////////////////////////////////
#include "Stntuple/obj/TStnInitDataBlock.hh"

//_____________________________________________________________________________
TStnInitDataBlock::TStnInitDataBlock() {
}


//_____________________________________________________________________________
TStnInitDataBlock::~TStnInitDataBlock() {
}

//-----------------------------------------------------------------------------
int TStnInitDataBlock::InitDataBlock(TStnDataBlock* Block, AbsEvent* Event, int Mode) {
  return -1;
}

//-----------------------------------------------------------------------------
int TStnInitDataBlock::ResolveLinks(TStnDataBlock* Block, AbsEvent* Event, int Mode) {
  return -1;
}

//-----------------------------------------------------------------------------
int TStnInitDataBlock::BeginRun(int RunNumber) {
  return 0;
}
