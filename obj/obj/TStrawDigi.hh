#ifndef __daqana_obj_TStrawDigi_hh__
#define __daqana_obj_TStrawDigi_hh__

#include <vector>
#include "TClonesArray.h"
#include "TObject.h"

class TStrawDigi : public TObject {
public:
  int    fNs;                   //     N(ADC samples)
  int    fSid;
  int    fMnid;                          // 
  int    fTdc0;
  int    fTdc1;
  int    fTot0;
  int    fTot1;
  int    fPmp;
  int    fFlag;
  int    fFs;
  float  fBl;
  float  fPh;
  std::vector<uint16_t> fAdc;
//-----------------------------------------------------------------------------
// functions
//-----------------------------------------------------------------------------
  TStrawDigi();
  TStrawDigi(int ns);
  virtual ~TStrawDigi();

  int     Ns() { return fNs; } // return adc.size(); }
  int     Init(int Ns);

  std::vector<uint16_t>& Adc() { return fAdc; }

  virtual void Clear(const char* Opt) override ;

  ClassDefOverride(TStrawDigi,1);
};

#endif
