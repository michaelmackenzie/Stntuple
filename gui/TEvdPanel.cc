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
#include "TObjArray.h"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"

#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"

#include "Offline/GeneralUtilities/inc/HepTransform.hh"
#include "Offline/TrackerGeom/inc/Straw.hh"

#include "Stntuple/gui/TEvdPanel.hh"
#include "Stntuple/gui/TEvdPlane.hh"
#include "Stntuple/gui/TEvdStraw.hh"
#include "Stntuple/gui/TStnVisManager.hh"
#include "Stntuple/gui/TStnGeoManager.hh"

#include "Offline/TrackerGeom/inc/Panel.hh"

using namespace CLHEP;

ClassImp(stntuple::TEvdPanel)

int stntuple::TEvdPanel::fgDebugLevel(0);

namespace stntuple {
//_____________________________________________________________________________
TEvdPanel::TEvdPanel(): TObject() {
  fVisible      = 0;
  fMnID         = -1;
}

//_____________________________________________________________________________
  TEvdPanel::TEvdPanel(int ID, const mu2e::Panel* Panel, TEvdPlane* Plane, const mu2e::Tracker* Tracker): TObject() {

  fID      = ID;
  fNLayers = Panel->nLayers();
  fPanel   = Panel;
  fVisible = 1;

  TStnGeoManager* gm = TStnGeoManager::Instance();
  const mu2e::TrkPanelMap::Row* tpm = gm->PanelMap(Plane->ID(),ID); // plane ID : 0-35 (???)
  if (tpm) fMnID              = tpm->mnid();

  int nst = Panel->nStraws();
  fListOfStraws = new TObjArray(nst);

  mu2e::StrawId pid = Panel->id();
  if (TEvdPanel::fgDebugLevel != 0) {
    printf("--- station, plane, panel: %3i %3i %3i\n",
           pid.getStation(),pid.getPlane(),pid.getPanel());
  }

  for (uint16_t ist=0; ist<nst; ist++) {
    const mu2e::Straw* straw = &Panel->getStraw(ist);

    int id = straw->id().asUint16();

    TEvdStraw* evd_straw = new TEvdStraw(id,straw,this, Tracker);
    fListOfStraws->Add(evd_straw);
  }
//-----------------------------------------------------------------------------
// build the transformation matrix
//-----------------------------------------------------------------------------
  const mu2e::HepTransform& ds_to_p = Panel->dsToPanel();
  //const HepRotation&  hr            = ds_to_p.rotation();
  //const Hep3Vector&   ht            = ds_to_p.displacement();

  // double phix = hr.phiX  ()*180./M_PI;
  // double phiy = hr.phiY  ()*180./M_PI;
  // double phiz = hr.phiZ  ()*180./M_PI;
  // double thtx = hr.thetaX()*180./M_PI;
  // double thty = hr.thetaY()*180./M_PI;
  // double thtz = hr.thetaZ()*180./M_PI;

  // double disp[3], disp1[3];
  // disp[0]  = ht.x();
  // disp[1]  = ht.y();
  // disp[2]  = ht.z();

  // // if (hr.zz() > 0) {
  // //   // thtx    = thtx+180;
  // //   // thty    = thty+180;
  // //   disp[0] = -disp[0];
  // //   disp[1] = -disp[1];
  // //   disp[2] = -disp[2];
  // // }

  // TGeoRotation* gr = new TGeoRotation(Form("rot_%02i_%i",pid.getPlane(),pid.getPanel()),thtx,phix,thty,phiy,thtz,phiz);
  // //  gr->SetMatrix(r);

  // gr->MasterToLocal(disp,disp1);

  // fCombiTrans     = new TGeoCombiTrans(Form("trk_%02i_%i",pid.getPlane(),pid.getPanel()),-disp1[0], -disp1[1], -disp1[2],gr);
  // //  fCombiTrans     = new TGeoCombiTrans(Form("trk_%02i_%i",pid.getPlane(),pid.getPanel()),0,0,0,gr);
  
  //  fPos.SetXYZ(disp[0],disp[1], disp[2]);
  fPos.SetXYZ(0,0,0); // fPos[3] is used in TStnVisManager.cc:OpenVRZView

  if (TEvdPanel::fgDebugLevel & 0x2) {
    // printf("panel fCombiTrans:\n");
    // fCombiTrans->Print();
    printf("mu2e::Panel HepTransform Panel->DS:\n");
    std::cout << Panel->panelToDS() << std::endl;

    printf("mu2e::Panel HepTransform DS->Panel:\n");
    std::cout << Panel->dsToPanel() << std::endl;
  }
}

//_____________________________________________________________________________
TEvdPanel::~TEvdPanel() {
}

//-----------------------------------------------------------------------------
void TEvdPanel::Paint(Option_t* Option) {


  int view = TVisManager::Instance()->GetCurrentView()->Type();

  if      (view == TStnVisManager::kXY) PaintXY (Option);
  else if (view == TStnVisManager::kRZ) PaintRZ (Option);
  else {
    // what is the default?
    //    Warning("Paint",Form("Unknown option %s",option));
  }

  gPad->Modified();
}


//-----------------------------------------------------------------------------
void TEvdPanel::PaintXY(Option_t* Option) {
}

//-----------------------------------------------------------------------------
void TEvdPanel::PaintRZ(Option_t* option) {
  // draw straws
}

//-----------------------------------------------------------------------------
  void TEvdPanel::PaintVST(Option_t* Option) {
  // draw straws
  int nstraws = NStraws();
  for (int is=0; is<nstraws; is++) {
    TEvdStraw* straw  = Straw(is);
    straw->PaintVST(Option);
  }
}

//-----------------------------------------------------------------------------
  void TEvdPanel::PaintVRZ(Option_t* Option) {
  // draw straws
  int nstraws = NStraws();
  for (int is=0; is<nstraws; is++) {
    TEvdStraw* straw  = Straw(is);
    straw->PaintVRZ(Option);
  }
}

//_____________________________________________________________________________
Int_t TEvdPanel::DistancetoPrimitive(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdPanel::DistancetoPrimitiveXY(Int_t px, Int_t py) {

  Int_t dist = 9999;

  static TVector3 global;
//   static TVector3 local;

  //  Double_t    dx1, dx2, dy1, dy2, dx_min, dy_min, dr;

  global.SetXYZ(gPad->AbsPixeltoX(px),gPad->AbsPixeltoY(py),0);

  return dist;
}

//_____________________________________________________________________________
Int_t TEvdPanel::DistancetoPrimitiveRZ(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
// panel is just a holder of straws, don't need to pick it
//-----------------------------------------------------------------------------
Int_t TEvdPanel::DistancetoPrimitiveVST(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdPanel::DistancetoPrimitiveVRZ(Int_t px, Int_t py) {
  int min_dist(9999);
  return min_dist;
}

}
