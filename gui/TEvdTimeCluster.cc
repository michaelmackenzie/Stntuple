///////////////////////////////////////////////////////////////////////////////
// Oct 2022 P.Murat
// 
//
// BaBar interface:
// ----------------
//      r     = fabs(1./om);
//      phi0  = Trk->helix(0.).phi0();
//      x0    =  -(1/om+d0)*sin(phi0);
//      y0    =   (1/om+d0)*cos(phi0);
//
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
#include "TPolyLine.h"
#include "TObjArray.h"

#include "art/Framework/Principal/Handle.h"

#include "Offline/RecoDataProducts/inc/HelixSeed.hh"

#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"

#include "Offline/TrackerGeom/inc/Tracker.hh"

#include "Stntuple/print/TAnaDump.hh"

#include "Stntuple/gui/TEvdTimeClusterVisNode.hh"
#include "Stntuple/gui/TEvdTimeCluster.hh"
#include "Stntuple/gui/TStnVisManager.hh"

// #include "CLHEP/Vector/ThreeVector.h"

ClassImp(stntuple::TEvdTimeCluster)

namespace stntuple {

//-----------------------------------------------------------------------------
TEvdTimeCluster::TEvdTimeCluster(): TObject() {
  // fListOfHits = NULL;
  fBox        = nullptr;
}

//-----------------------------------------------------------------------------
// pointer to track is just cached
//-----------------------------------------------------------------------------
  TEvdTimeCluster::TEvdTimeCluster(int Number,
                                   const mu2e::TimeCluster* TimeCluster,
                                   const mu2e::ComboHitCollection* ChColl,
                                   float T0,
				   float TMin, float TMax, float ZMin, float ZMax, 
                                   float PhiMin, float PhiMax,
                                   TStnVisNode* VisNode): TObject() {
  fNumber      = Number;
  fTimeCluster = TimeCluster;
  fChColl      = ChColl;
  fVisNode     = VisNode;

  fT0     = T0;
  fTMin   = TMin;
  fTMax   = TMax;
  fZMin   = ZMin;
  fZMax   = ZMax;
  fPhiMin = PhiMin;
  fPhiMax = PhiMax;

  fBox  = new TBox(ZMin,TMin,ZMax,TMax);
  fBox->SetLineColor(kBlue-7);
  fBox->SetFillColor(kBlue-7);
  fBox->SetFillStyle(3002);

  fListOfHits = nullptr;

  float rmax = 700.;

  fEllipse = new TEllipse(0.,0.,rmax,rmax,fPhiMin*180/M_PI,fPhiMax*180/M_PI,0);
  fEllipse->SetLineColor(kGreen);
  fEllipse->SetFillColor(kBlue+1);
  fEllipse->SetFillStyle(3003);

}

//-----------------------------------------------------------------------------
TEvdTimeCluster::~TEvdTimeCluster() {
  if (fBox) delete fBox;

  if (fListOfHits) {
    fListOfHits->Delete();
    delete fListOfHits;
  }
}

//-----------------------------------------------------------------------------
// Time clusters are displayed only in TZ view and used for hit selection in 
// other views
//-----------------------------------------------------------------------------
void TEvdTimeCluster::Paint(Option_t* Option) {

  int view = TVisManager::Instance()->GetCurrentView()->Type();

  if      (view == TStnVisManager::kXY ) PaintXY (Option);
  else if (view == TStnVisManager::kTZ ) PaintTZ (Option);

  gPad->Modified();
}

//-----------------------------------------------------------------------------
void TEvdTimeCluster::PaintXY(Option_t* Option) {
  fEllipse->Paint(Option);
}

//-----------------------------------------------------------------------------
// make sure the time cluster width is never too small to click on it
// 6 pixels should be enough
//-----------------------------------------------------------------------------
void TEvdTimeCluster::PaintTZ(Option_t* Option) {

  int py1  = gPad->YtoAbsPixel(fTMin);
  int py2  = gPad->YtoAbsPixel(fTMax);
  if (fabs(py1-py2) < 6) {
                                        // scale of the view
    double tmean = (fTMin+fTMax)/2.;
    py1  = gPad->YtoAbsPixel(tmean-1000);
    py2  = gPad->YtoAbsPixel(tmean+1000);
    double scale = 2000/(fabs(py2-py1)+1.e-12);
    double t1    = (fTMin+fTMax)/2.-3*scale;
    double t2    = (fTMin+fTMax)/2.+3*scale;
    fBox->SetY1(t1);
    fBox->SetY2(t2);
  }

  fBox->Paint(Option);
}

//_____________________________________________________________________________
  Int_t TEvdTimeCluster::DistancetoPrimitive(Int_t px, Int_t py) {
  return 9999;
}

//-----------------------------------------------------------------------------
// select based either on an upper edge or a lower edge
// 5 is a magic number - see TStnView.cc - a threshold above which a closest object 
// is not defined
// note Y on the screen increases from the top to the bottom, so y1 > y2
//-----------------------------------------------------------------------------
int TEvdTimeCluster::DistancetoPrimitiveTZ(Int_t px, Int_t py) {
  int dist(9999);

  int y1  = gPad->YtoAbsPixel(fTMin);
  int y2  = gPad->YtoAbsPixel(fTMax);

  if ((py <= y1) and (py >= y2)) dist = 5;

  return dist;
}


//-----------------------------------------------------------------------------
void TEvdTimeCluster::Clear(Option_t* Opt) {
  //  SetLineColor(1);
}


//-----------------------------------------------------------------------------
// interactively, always print everything
//-----------------------------------------------------------------------------
void TEvdTimeCluster::Print(Option_t* Option) const {

  TAnaDump* ad = TAnaDump::Instance();

  TEvdTimeClusterVisNode* vn = (TEvdTimeClusterVisNode*) fVisNode;

  ad->printTimeCluster(fTimeCluster,"data+banner+hits",vn->ChColl(), vn->SdmcCollTag().encode().data());
}

//-----------------------------------------------------------------------------
  void TEvdTimeCluster::PrintMe() const {
    Print("");
  }

//-----------------------------------------------------------------------------
void TEvdTimeCluster::ExecuteEvent(int Event, int Px, int Py) {

  // switch (Event) {
  // case kButton1Down: 
  //   Select();
  // }
}

//-----------------------------------------------------------------------------
void TEvdTimeCluster::Select() {

  TStnVisManager* vm = TStnVisManager::Instance();

  TEvdTimeCluster* selected = vm->SelectedTimeCluster();

  if (selected == this) { 
//-----------------------------------------------------------------------------
// deselect previously selected cluster
//-----------------------------------------------------------------------------
    vm->SetSelectedTimeCluster(nullptr);
    fBox->SetLineColor(kBlue-7);
    fBox->SetFillColor(kBlue-7);
  }
  else {
//-----------------------------------------------------------------------------
// previously selected cluster was different: deselect it
//-----------------------------------------------------------------------------
    if (selected != nullptr) {
      selected->Box()->SetLineColor(kBlue-7);
      selected->Box()->SetFillColor(kBlue-7);
    }
    vm->SetSelectedTimeCluster(this);
    fBox->SetLineColor(kRed+2);
    fBox->SetFillColor(kRed+2);
  }
  
}
//-----------------------------------------------------------------------------
// check if a hit with 'Index' in the fShColl collection belongs to the time cluster
// time cluster is made out of ComboHit's
// the code could be made a bit more efficient for combo hits at a cost of passing the 
// the hit type explicitly
//-----------------------------------------------------------------------------
  int TEvdTimeCluster::TCHit(int Index) {
  int ok(0);

  int nch = TimeCluster()->nhits();
  for (int i=0; i<nch; i++) {
    int ind = TimeCluster()->hits().at(i);
    const mu2e::ComboHit* ch = &fChColl->at(ind);
    int nsh = ch->nStrawHits();
    for (int ish=0; ish<nsh; ish++) {
      if (ch->index(ish) == Index) {
	ok = 1;
	break;
      }
    }
  }

  return ok;
}


}
