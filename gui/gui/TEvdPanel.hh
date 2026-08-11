///////////////////////////////////////////////////////////////////////////////
// vis node 
///////////////////////////////////////////////////////////////////////////////
#ifndef Stntuple_gui_TEvdPanel_hh
#define Stntuple_gui_TEvdPanel_hh

#include "Gtypes.h"
#include "TObjArray.h"
#include "TVector3.h"
// #include "TGeoMatrix.h"

namespace mu2e {
  class Panel;
  class Tracker;
}

namespace stntuple {

class TEvdPlane;
class TEvdStraw;

class TEvdPanel: public TObject {
public:
  
protected:
  int                fID;
  int                fMnID;             // panel Minnesota ID
  int                fVisible;
  int                fNLayers;
  TObjArray*         fListOfStraws;
  TVector3           fPos;

  TEvdPlane*         fPlane; 		// backward pointer
  const mu2e::Panel* fPanel;

  //  TGeoCombiTrans*    fCombiTrans;       // rotate, then translate

  static int         fgDebugLevel;
public:
//-----------------------------------------------------------------------------
// constructors and destructor
//-----------------------------------------------------------------------------
  TEvdPanel();
  TEvdPanel(int Number, const mu2e::Panel* Panel, TEvdPlane* Plane, const mu2e::Tracker* Tracker); 

  virtual ~TEvdPanel();
//-----------------------------------------------------------------------------
// accessors
//-----------------------------------------------------------------------------
//  TGeoCombiTrans* GetCombiTrans() { return fCombiTrans; }
  int             NLayers      () { return fNLayers;    }
  int             MnID         () { return fMnID;       }
  int             NStraws      () { return fListOfStraws->GetEntriesFast(); }
  int             Visible      () { return fVisible;    }

  TEvdStraw* Straw  (int I) { return (TEvdStraw*) fListOfStraws->UncheckedAt(I); }

  TVector3*  Pos() { return &fPos; };         // position of the panel center

  const mu2e::Panel* GetMu2ePanel() { return fPanel; }
//-----------------------------------------------------------------------------
// modifiers
//-----------------------------------------------------------------------------
  static void SetDebugLevel(int Level) { fgDebugLevel = Level; }
  
  void SetVisible(int YesNo) { fVisible = YesNo; }
  void SetMnID   (int ID   ) { fMnID    = ID;    }
  
  //  virtual void  Draw    (Option_t* option = "");

  virtual void  Paint   (Option_t* option = "");
          void  PaintXY (Option_t* option = "");
          void  PaintRZ (Option_t* option = "");
          void  PaintVST(Option_t* option = "");
          void  PaintVRZ(Option_t* option = "");

  //  virtual void  ExecuteEvent(Int_t event, Int_t px, Int_t py);

  virtual Int_t DistancetoPrimitive   (Int_t px, Int_t py);
  virtual Int_t DistancetoPrimitiveXY (Int_t px, Int_t py);
  virtual Int_t DistancetoPrimitiveRZ (Int_t px, Int_t py);
  virtual Int_t DistancetoPrimitiveVST(Int_t px, Int_t py);
  virtual Int_t DistancetoPrimitiveVRZ(Int_t px, Int_t py);

  //  virtual void   Print(const char* Opt = "") const ; // **MENU**

  ClassDef(stntuple::TEvdPanel,0)
};

}
#endif
