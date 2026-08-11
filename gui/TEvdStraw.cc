///////////////////////////////////////////////////////////////////////////////
// May 04 2013 P.Murat
// 
// in 'XY' mode draw calorimeter clusters as circles with different colors 
// in 'Cal' mode draw every detail...
///////////////////////////////////////////////////////////////////////////////
#include <format>

#include "TVirtualX.h"
#include "TPad.h"
#include "TStyle.h"
#include "TVector3.h"
#include "TLine.h"
#include "TArc.h"
#include "TArrow.h"
#include "TMath.h"
#include "TBox.h"
#include "TObjArray.h"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"

#include "Offline/TrackerGeom/inc/Straw.hh"
#include "Offline/TrackerGeom/inc/Tracker.hh"
#include "Offline/RecoDataProducts/inc/StrawDigi.hh"

#include "Stntuple/gui/TEvdPanel.hh"
#include "Stntuple/gui/TEvdStraw.hh"
#include "Stntuple/gui/TEvdStrawHit.hh"
#include "Stntuple/gui/TEvdTimeCluster.hh"
#include "Stntuple/gui/TStnVisManager.hh"

ClassImp(stntuple::TEvdStraw)

namespace stntuple {
//_____________________________________________________________________________
TEvdStraw::TEvdStraw(): TObject() {
  fListOfHits = new TObjArray();
}

//-----------------------------------------------------------------------------
// drawing straws makes sense only on RZ view, in any other view it is just 
// a waste of time ... and screen space
//-----------------------------------------------------------------------------
  TEvdStraw::TEvdStraw(int Index, const mu2e::Straw* Straw, TEvdPanel* Panel,
                       const mu2e::Tracker* Tracker):  TObject()  {
  fIndex      = Index;
  fStraw      = Straw;
  fPanel      = Panel;
  fHalfLength = fStraw->halfLength();

  const CLHEP::Hep3Vector& pos = fStraw->getMidPoint  ();
  const CLHEP::Hep3Vector& dir = fStraw->wireDirection();

  fPos.SetXYZ(pos.x(),pos.y(),pos.z());
  fDir.SetXYZ(dir.x(),dir.y(),dir.z());

  double z     = fPos.Z();
  double rwire = pos.perp();                      // radial position of the wire
  double r     = Tracker->strawProperties()._strawOuterRadius;

  fArc = new TArc(z,rwire,r);

  double x1,y1, x2,y2;

  x1 = fPos.X()-fHalfLength*fDir.X();
  y1 = fPos.Y()-fHalfLength*fDir.Y();

  x2 = fPos.X()+fHalfLength*fDir.X();
  y2 = fPos.Y()+fHalfLength*fDir.Y();

  fWire = new TLine(x1,y1,x2,y2);

  fListOfHits = new TObjArray(5);
  fListOfHits->SetOwner(kFALSE);
}

//_____________________________________________________________________________
TEvdStraw::~TEvdStraw() {

  fListOfHits->Clear();
  delete fListOfHits;

  delete fArc;
}

//-----------------------------------------------------------------------------
void TEvdStraw::Paint(Option_t* option) {
  // paints one disk (.. or vane, in the past), i.e. section

  int view = TVisManager::Instance()->GetCurrentView()->Type();

  if      (view == TStnVisManager::kXY) PaintXY (option);
  else if (view == TStnVisManager::kRZ) PaintRZ (option);
  else {
    // what is the default?
    //    Warning("Paint",Form("Unknown option %s",option));
  }

  gPad->Modified();
}


//_____________________________________________________________________________
void TEvdStraw::PaintXY(Option_t* Option) {
}


//_____________________________________________________________________________
void TEvdStraw::PaintRZ(Option_t* Option) {
  // draw straw

  int    nhits, color(0), style(0);
  double xstraw, ystraw, x1, x2, y1, y2;

  TStnVisManager* vm = TStnVisManager::Instance();
  float tmin = vm->TMin(); 
  float tmax = vm->TMax();
  stntuple::TEvdTimeCluster* etcl = vm->SelectedTimeCluster();
  if (etcl) {
    tmin = etcl->TMin(); // FIXME!
    tmax = etcl->TMax(); // FIXME!
  }
//-----------------------------------------------------------------------------
// do not even attempt to paint straws outside the visible range
//-----------------------------------------------------------------------------
  gPad->GetRange(x1,y1,x2,y2);

  xstraw = fArc->GetX1();
  ystraw = fArc->GetY1();

  if ((xstraw < x1) || (xstraw > x2) || (ystraw < y1) || (ystraw > y2)) return;

  nhits = fListOfHits->GetEntriesFast();
//-----------------------------------------------------------------------------
// withour a reconstructed track, 
// the straw hit drift time is unknown, so just change the color of the straw circle
//-----------------------------------------------------------------------------
  fArc->SetLineColor(1);
  fArc->SetLineWidth(1);

  for (int i=0; i<nhits; ++i) {
    stntuple::TEvdStrawHit* evd_sh = Hit(i);
    const mu2e::ComboHit*   sch    = evd_sh->StrawHit();
    float t = sch->correctedTime();
    if ((t >= tmin) and (t <= tmax)) {    
      int ok = 1;
      if (etcl and vm->DisplayOnlyTCHits()) { 
        ok = etcl->TCHit(sch->index());
      }
      if (ok) {
        color = kRed;
        fArc->SetLineColor(color+1);
        fArc->SetLineWidth(2);
      }
    }
  }

  fArc->SetFillStyle(style);
  fArc->Paint(Option);
//-----------------------------------------------------------------------------
// not sure why this is here - the drift time still is not known and one cant do
// better than just mark the straw as "a straw with hits" (done above)
//-----------------------------------------------------------------------------
  // if (vm->DisplayStrawDigiMC()) {
  //   for (int i=0; i<nhits; i++) {
  //     Hit(i)->PaintRZ(Option);
  //   }
  //  }
}

//_____________________________________________________________________________
void TEvdStraw::PaintVST(Option_t* Option) {
  double x1,y1, x2,y2;
  gPad->GetRange(x1,y1,x2,y2);

  fWire->Paint(Option);
}

//-----------------------------------------------------------------------------
// VRZ view of the straw is the same, just keep the namning scheme - in the local coordinate frame of the panel
//-----------------------------------------------------------------------------
void TEvdStraw::PaintVRZ(Option_t* Option) {
  int    nhits, color(0), style(0);
  double xstraw, ystraw, x1, x2, y1, y2;

  TStnVisManager* vm = TStnVisManager::Instance();
  float tmin = vm->TMin(); 
  float tmax = vm->TMax();
  stntuple::TEvdTimeCluster* etcl = vm->SelectedTimeCluster();
  if (etcl) {
    tmin = etcl->TMin(); // FIXME!
    tmax = etcl->TMax(); // FIXME!
  }
//-----------------------------------------------------------------------------
// do not even attempt to paint straws outside the visible range
//-----------------------------------------------------------------------------
  gPad->GetRange(x1,y1,x2,y2);

  // find local coordinate
  // TGeoCombiTrans* gt = vm->GetCurrentView()->GetCombiTrans();

  // double posm[3], posl[3];
  // posm[0] = fStraw->getMidPoint().x();
  // posm[1] = fStraw->getMidPoint().y();
  // posm[2] = fStraw->getMidPoint().z();
  // gt->MasterToLocalVect(posm, posl);

  const mu2e::Panel* panel     = (const mu2e::Panel*) vm->GetCurrentView()->GetMother();
  const mu2e::HepTransform& ht = panel->dsToPanel();
  CLHEP::Hep3Vector pos_l = ht*fStraw->getMidPoint();

  xstraw = pos_l[2];
  ystraw = pos_l[1];

  if ((xstraw < x1) || (xstraw > x2) || (ystraw < y1) || (ystraw > y2)) return;

  nhits = fListOfHits->GetEntriesFast();
//-----------------------------------------------------------------------------
// withour a reconstructed track, 
// the straw hit drift time is unknown, so just change the color of the straw circle
//-----------------------------------------------------------------------------
  TArc arc(*fArc);
  arc.SetX1(xstraw);
  arc.SetY1(ystraw);
  arc.SetLineColor(1);
  arc.SetLineWidth(1);

  for (int i=0; i<nhits; ++i) {
    stntuple::TEvdStrawHit* evd_sh = Hit(i);
    const mu2e::ComboHit*   sch    = evd_sh->StrawHit();
    float t = sch->correctedTime();
    if ((t >= tmin) and (t <= tmax)) {    
      int ok = 1;
      if (etcl and vm->DisplayOnlyTCHits()) { 
        ok = etcl->TCHit(sch->index());
      }
      if (ok) {
        color = kRed;
        arc.SetLineColor(color+1);
        arc.SetLineWidth(2);
      }
    }
  }

  arc.SetFillStyle(style);
  arc.Paint(Option);
}

//_____________________________________________________________________________
Int_t TEvdStraw::DistancetoPrimitive(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdStraw::DistancetoPrimitiveXY(Int_t px, Int_t py) {

  Int_t dist = 9999;

  static TVector3 global;
//   static TVector3 local;

  //  Double_t    dx1, dx2, dy1, dy2, dx_min, dy_min, dr;

  global.SetXYZ(gPad->AbsPixeltoX(px),gPad->AbsPixeltoY(py),0);

  return dist;
}

//_____________________________________________________________________________
Int_t TEvdStraw::DistancetoPrimitiveRZ(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdStraw::DistancetoPrimitiveVST(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdStraw::DistancetoPrimitiveVRZ(Int_t Cx, Int_t Cy) {
  TVector3 global;

  // convert coordinates of the wire center to the local frame
  const CLHEP::Hep3Vector& pos_ds = fStraw->getMidPoint  ();
  CLHEP::Hep3Vector posl          = fPanel->GetMu2ePanel()->dsToPanel()*pos_ds;

  double cx_l = gPad->PixeltoX(Cx);
  double cy_l = gPad->PixeltoY(Cy);
  double dx   = cx_l-posl.z();
  double dy   = cy_l-posl.y();
  double dist = sqrt(dx*dx+dy*dy);

  int idist = abs(Cx-gPad->XtoPixel(cx_l+dist));
  
  if (dist < 2.5) {
    Print();
    std::cout << std::format("dist:{} idist:{}\n",dist,idist);
  }
  
  return idist;
}


//-----------------------------------------------------------------------------
void TEvdStraw::Clear(const char* Opt) {
  fListOfHits->Clear();
}

//-----------------------------------------------------------------------------
void TEvdStraw::Print(const char* Opt) const {
  const CLHEP::Hep3Vector& pos_ds = fStraw->getMidPoint  ();
  CLHEP::Hep3Vector posl          = fPanel->GetMu2ePanel()->dsToPanel()*pos_ds;

  std::cout << std::format(" plane:{} panel:{} straw:{} (wx,wy): ({:10.3f}{:10.3f}) nhits:{:2d}\n",
                         fStraw->id().plane(),fStraw->id().panel(),fStraw->id().straw(),
                           posl.z(),posl.y(),fListOfHits->GetEntriesFast());
}
  
}
