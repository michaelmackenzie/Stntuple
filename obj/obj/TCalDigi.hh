#ifndef __daqana_obj_TCalDigi_hh__
#define __daqana_obj_TCalDigi_hh__

#include <vector>
#include "TClonesArray.h"
#include "TObject.h"

class TCalDigi : public TObject {
public:
  int    fNs;
  int    fSipmID;
  int    fT0;
  int    fPpos;                          // peak position
  std::vector<uint16_t> fWf;
//-----------------------------------------------------------------------------
// functions
//-----------------------------------------------------------------------------
  TCalDigi();
  TCalDigi(int ns);
  virtual ~TCalDigi();

  int     Ns() { return fNs; } // return adc.size(); }
  int     Init(int Ns);

  void    Set(int SipmID, float T0, float PeakPos, const std::vector<int>* Wf);

  std::vector<uint16_t>& Wf() { return fWf; }

  virtual void Clear(const char* Opt) override ;

  ClassDefOverride(TCalDigi,1);
};

#endif
