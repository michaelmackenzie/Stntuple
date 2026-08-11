///////////////////////////////////////////////////////////////////////////////
// May 04 2013 P.Murat
// 
// in 'XY' mode draw calorimeter clusters as circles with different colors 
// in 'Cal' mode draw every detail...
///////////////////////////////////////////////////////////////////////////////
#include "TVirtualX.h"
#include "TPad.h"
#include "TStyle.h"
#include "TVector3.h"
#include "TLine.h"
#include "TArc.h"
#include "TArrow.h"
#include "TMath.h"
#include "TBox.h"
#include "TEllipse.h"
#include "TObjArray.h"

#include "art/Framework/Principal/Handle.h"

#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"
#include "Offline/TrackerGeom/inc/Tracker.hh"

#include "Stntuple/gui/TEvdStraw.hh"
#include "Stntuple/gui/TEvdTrkStrawHit.hh"
#include "Stntuple/gui/TStnVisManager.hh"

ClassImp(stntuple::TEvdTrkStrawHit)

namespace stntuple {
//_____________________________________________________________________________
  TEvdTrkStrawHit::TEvdTrkStrawHit(const mu2e::TrkStrawHitSeed* Tshs, TEvdStraw* Straw): TObject() {
  fTshs = Tshs;
  fStraw = Straw;

  fLineW.SetX1(fPos.X()-fStrawDir.X()*fSigW);
  fLineW.SetY1(fPos.Y()-fStrawDir.Y()*fSigW);
  fLineW.SetX2(fPos.X()+fStrawDir.X()*fSigW);
  fLineW.SetY2(fPos.Y()+fStrawDir.Y()*fSigW);

  fLineR.SetX1(fPos.X()+fStrawDir.Y()*fSigR);
  fLineR.SetY1(fPos.Y()-fStrawDir.X()*fSigR);
  fLineR.SetX2(fPos.X()-fStrawDir.Y()*fSigR);
  fLineR.SetY2(fPos.Y()+fStrawDir.X()*fSigR);

  const mu2e::Straw* s = fStraw->GetStraw();
  double zw     = s->getMidPoint().z();
  double rw     = s->getMidPoint().perp();

  double rdrift = fTshs->driftRadius(); // 4.0 ; //

  fEllipse.SetX1(zw);
  fEllipse.SetY1(rw);
  fEllipse.SetR1(rdrift);
  fEllipse.SetR2(rdrift);
  fEllipse.SetFillStyle(3001);
  fEllipse.SetFillColor(kBlue+2);

  if (fTshs->ambig() == 0) fEllipse.SetFillStyle(3003);
}

//-----------------------------------------------------------------------------
TEvdTrkStrawHit::~TEvdTrkStrawHit() {
}

//-----------------------------------------------------------------------------
void TEvdTrkStrawHit::Paint(Option_t* Option) {
  const char oname[] = "TEvdTrkStrawHit::Paint";

  int view = TVisManager::Instance()->GetCurrentView()->Type();

  if      (view == TStnVisManager::kXY ) PaintXY (Option);
  else if (view == TStnVisManager::kRZ ) PaintRZ (Option);
  else if (view == TStnVisManager::kCal) PaintCal(Option);
  else {
    printf("[%s] >>> ERROR: unknown view: %i, DO NOTHING\n",oname,view);
  }

  gPad->Modified();
}

//_____________________________________________________________________________
void TEvdTrkStrawHit::PaintXY(Option_t* Option) {
  fLineW.Paint(Option);
  fLineR.Paint(Option);
}

//_____________________________________________________________________________
void TEvdTrkStrawHit::PaintRZ(Option_t* Option) {

  if (fTshs->flag().hasAllProperties(mu2e::StrawHitFlag::active)) {
    fEllipse.SetFillColor(kBlue-10);
  }
  else {
    fEllipse.SetFillColor(kRed-10);
  }
  fEllipse.SetFillStyle(3001);
  fEllipse.Paint(Option);
}

//_____________________________________________________________________________
void TEvdTrkStrawHit::PaintVRZ(Option_t* Option) {
  TEllipse e(fEllipse);

  TStnVisManager* vm = TStnVisManager::Instance();
  const mu2e::Panel* panel = (const mu2e::Panel*) vm->GetCurrentView()->GetMother();
  const mu2e::HepTransform&   ht = panel->dsToPanel();

  CLHEP::Hep3Vector pos_l = ht*fStraw->GetStraw()->getMidPoint();
  e.SetX1(pos_l[2]);
  e.SetY1(pos_l[1]);

  if (fTshs->flag().hasAllProperties(mu2e::StrawHitFlag::active)) {
    e.SetFillColor(kBlue-10);
  }
  else {
    e.SetFillColor(kRed-10);
  }

  e.SetFillStyle(3001);
  e.Paint(Option);
}

//_____________________________________________________________________________
void TEvdTrkStrawHit::PaintCal(Option_t* option) {
  // nothing to draw...
}


//_____________________________________________________________________________
Int_t TEvdTrkStrawHit::DistancetoPrimitive(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdTrkStrawHit::DistancetoPrimitiveXY(Int_t px, Int_t py) {

  Int_t dist = 9999;

  static TVector3 global;

  global.SetXYZ(gPad->AbsPixeltoX(px),gPad->AbsPixeltoY(py),0);

  return dist;
}

//_____________________________________________________________________________
Int_t TEvdTrkStrawHit::DistancetoPrimitiveRZ(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdTrkStrawHit::DistancetoPrimitiveVRZ(Int_t px, Int_t py) {
  return 9999;
}

}
