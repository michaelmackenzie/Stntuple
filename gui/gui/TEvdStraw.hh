///////////////////////////////////////////////////////////////////////////////
// vis node displays one wedge
///////////////////////////////////////////////////////////////////////////////
#ifndef TEvdStraw_hh
#define TEvdStraw_hh

#include "Gtypes.h"
#include "TClonesArray.h"
#include "TH1.h"
#include "TPad.h"
// #include "TArc.h"
// #include "TLine.h"
#include "TVector3.h"

namespace mu2e {
  class Straw;
  class Tracker;
}

class TLine;
class TArc;

namespace stntuple {

class TEvdStation;
class TEvdStraw;
class TEvdStrawHit;
class TEvdPanel;

class TEvdStraw: public TObject {
public:
  
protected:
  int                fIndex;
  //  int                fNHits;
  TObjArray*         fListOfHits;       // do we need it ? 

  TEvdPanel*         fPanel; 		// backward pointer
  TVector3           fPos;
  TVector3           fDir;              // forgetting mis-alignment, direction is the same
                                        // for all straws of the panel
  float              fHalfLength;

  TArc*              fArc;              // circle ...
  TLine*             fWire;             // line in XY

  const mu2e::Straw* fStraw;
public:
//-----------------------------------------------------------------------------
// constructors and destructor
//-----------------------------------------------------------------------------
  TEvdStraw();
  TEvdStraw(int Index, const mu2e::Straw* Straw, TEvdPanel* Panel, const mu2e::Tracker* Tracker); 

  virtual ~TEvdStraw();
//-----------------------------------------------------------------------------
// accessors
//-----------------------------------------------------------------------------
  int NHits() { return fListOfHits->GetEntriesFast(); }

  TEvdStrawHit*   Hit(int I) { 
    return (TEvdStrawHit*) fListOfHits->UncheckedAt(I); 
  }
                                        // center of the straw, for displaying
  TVector3*  Pos() { return &fPos; }

  const mu2e::Straw*   GetStraw() { return fStraw; }
//-----------------------------------------------------------------------------
// modifiers
//-----------------------------------------------------------------------------
  void AddHit(TObject* Hit) { fListOfHits->Add(Hit); }
//-----------------------------------------------------------------------------
// other methods
//-----------------------------------------------------------------------------
  virtual void  Paint   (Option_t* option = "");
  virtual void  PaintXY (Option_t* option = "");
  virtual void  PaintRZ (Option_t* option = "");
  virtual void  PaintVST(Option_t* option = "");
  virtual void  PaintVRZ(Option_t* option = "");

  //  virtual void  ExecuteEvent(Int_t event, Int_t px, Int_t py);

  virtual Int_t DistancetoPrimitive   (Int_t px, Int_t py);
  virtual Int_t DistancetoPrimitiveXY (Int_t px, Int_t py);
  virtual Int_t DistancetoPrimitiveRZ (Int_t px, Int_t py);
  virtual Int_t DistancetoPrimitiveVST(Int_t px, Int_t py);
  virtual Int_t DistancetoPrimitiveVRZ(Int_t px, Int_t py);

  virtual void Clear(const char* Opt = "")       ;
  virtual void Print(const char* Opt = "") const ; // **MENU**

  ClassDef(stntuple::TEvdStraw,0)
};

}
#endif
