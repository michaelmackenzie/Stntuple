//-----------------------------------------------------------------------------
//  2014-01-26 P.Murat - TStrawHit
//-----------------------------------------------------------------------------
#ifndef TStrawHit_hh
#define TStrawHit_hh

#include <math.h>
#include "TMath.h"
#include "TObject.h"
#include "TBuffer.h"

#include "Offline/DataProducts/inc/StrawId.hh"

class TStrawHit : public TObject {
public:
					// data
  int     fStrawID;			// sid | (mcflag << 16)
					// -------------------- MC info, don't write out for the data
  int     fGenID;                       // generator ID + StrawDigiFlag << 16
  int     fSimID;			// sim particle ID
  int     fPdgID;			// sim particle PDG ID
  int     fMotherPdgID;	                // mother PDG ID
	  				// ----------------------------- floats
  float   fTime[2];			// 
  float   fTOT[2];                      // 
  float   fEDep;			// energy deposition for the hit
  float   fMcMom;			// MC particle momentum

public:
                                        // constructors and destructors
  TStrawHit(int I = -1);
  virtual ~TStrawHit();
//-----------------------------------------------------------------------------
// accessors
//-----------------------------------------------------------------------------
  int   StrawID    () const { return (fStrawID & 0x0000ffff )               ; }
  int   MCFlag     () const { return (fStrawID & 0xffff0000 ) >> 16         ; }
		     
  int   Station    () const { return (fStrawID & mu2e::StrawId::_stationmsk) >> mu2e::StrawId::_stationsft; }
  int   Panel      () const { return (fStrawID & mu2e::StrawId::_panelmsk  ) >> mu2e::StrawId::_panelsft  ; }
  int   Face       () const { return (fStrawID & mu2e::StrawId::_facemsk   ) >> mu2e::StrawId::_facesft   ; }
  int   Layer      () const { return (fStrawID & mu2e::StrawId::_layermsk  )               ; }
  int   Straw      () const { return (fStrawID & mu2e::StrawId::_strawmsk  )               ; }
  int   Preamp     () const { return (fStrawID & mu2e::StrawId::_preampmsk ) >> mu2e::StrawId::_preampsft ; }

  float Time  (int I) const { return fTime[I]; }
  float TOT   (int I) const { return fTOT [I]; }
  int   Dt         () const { return fTime[1]-fTime[0]; }

  int   PdgID      () const { return fPdgID;       }
  int   MotherPdgID() const { return fMotherPdgID   ; }
  int   GenID      () const { return fGenID & 0XFFFF; }
  int   SimID      () const { return fSimID;       }

					// just one byte
  int   StrawDigiFlag () const { return (fGenID >> 16) & 0xFF; }

  float EDep       () const { return fEDep;        }
  float McMom      () const { return fMcMom;       }
//-----------------------------------------------------------------------------
// modifiers, assume TOT = tot[0] | (tot[1] << 16
//-----------------------------------------------------------------------------
  void    Set(int   StrawID, float* Time , float* TOT  , 
	      int   GenID  , int    SimID, 
	      int   PdgID  , int    MotherPdgID, 
	      float EDep   , float  McMom);
//-----------------------------------------------------------------------------
// overloaded methods of TObject
//-----------------------------------------------------------------------------
  void Clear(Option_t* opt = "");
  void Print(Option_t* opt = "") const;
//-----------------------------------------------------------------------------
// schema evolution
//-----------------------------------------------------------------------------
  void ReadV1(TBuffer& R__b);
  void ReadV2(TBuffer& R__b);

  ClassDef (TStrawHit,3)
};

#endif
