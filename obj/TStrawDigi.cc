//

#include "TBuffer.h"
#include "Stntuple/obj/TStrawDigi.hh"

ClassImp(TStrawDigi)

//-----------------------------------------------------------------------------
void TStrawDigi::Streamer(TBuffer& R__b) {
  
  int nwi = ((int*) &fBl   ) - &fNs;
  int nwf = ((float*) &fAdc) - &fBl;

  if (R__b.IsReading()) {
    // Version_t R__v = R__b.ReadVersion();  // not used but want to look at 
    
    // if      (R__v == 1) ReadV1(R__b);
    // else if (R__v == 2) ReadV2(R__b);
    // else {
                                        // current version: V3
    R__b.ReadFastArray(&fNs, nwi);
    R__b.ReadFastArray(&fBl, nwf);
    if (fNs != int(fAdc.size())) {
      fAdc.resize(fNs);
    }
    if (fNs != 0) {
      R__b.ReadFastArray(fAdc.data(),fNs);
    }
  }
  else {
    R__b.WriteVersion(TStrawDigi::IsA());

    R__b.WriteFastArray(&fNs,nwi);
    R__b.WriteFastArray(&fBl,nwf);
    if (fNs != 0) {
      R__b.WriteFastArray(fAdc.data(),fNs);
    }
  }
}

//-----------------------------------------------------------------------------
// std::vector initializes to empty
//-----------------------------------------------------------------------------
TStrawDigi::TStrawDigi() : TObject(), fNs(0), fSid(-1), fMnid(-1), fTdc0(-1), fTdc1(-1),
                           fTot0(-1), fTot1(-1), fPmp(-1), fFlag(-1), fBl(-1.), fPh(-1.)
{
}

//-----------------------------------------------------------------------------
TStrawDigi::~TStrawDigi() {
}

//-----------------------------------------------------------------------------
int TStrawDigi::Init(int Ns) {
  int rc(0);
  if (fNs != Ns) {
    fNs = Ns;
    fAdc.resize(fNs);
  }
  return rc;
}

//-----------------------------------------------------------------------------
void TStrawDigi::Clear(const char* Opt) {
}
