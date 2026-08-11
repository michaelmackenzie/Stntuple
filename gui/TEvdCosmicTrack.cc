///////////////////////////////////////////////////////////////////////////////
// May 04 2013 P.Murat
// 
// in 'XY' mode draw calorimeter clusters as circles with different colors 
// in 'Cal' mode draw every detail...
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



#include "Offline/RecoDataProducts/inc/CosmicTrackSeed.hh"

#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"

#include "Offline/TrackerGeom/inc/Tracker.hh"

#include "Stntuple/gui/TEvdCosmicTrack.hh"
#include "Stntuple/gui/TStnVisManager.hh"
#include "Stntuple/gui/TEvdTrkStrawHit.hh"
#include "Stntuple/base/TObjHandle.hh"

#include "CLHEP/Vector/ThreeVector.h"
#include "gui/TEvdCosmicTrack.hh"

ClassImp(stntuple::TEvdCosmicTrack)

namespace stntuple {

//-----------------------------------------------------------------------------
TEvdCosmicTrack::TEvdCosmicTrack(): TObject() {
  fCTSeed     = nullptr;
  fListOfHits = nullptr;
  fLineXY     = nullptr;
  fLineZY     = nullptr;
}

//-----------------------------------------------------------------------------
// pointer to track is just cached
//-----------------------------------------------------------------------------
  TEvdCosmicTrack::TEvdCosmicTrack(int Number, const mu2e::CosmicTrackSeed* CTSeed): TObject() {
  fNumber  = Number;
  fCTSeed  = CTSeed;

  fListOfHits = new TObjArray();
  fListOfHits->SetOwner(kTRUE);

  fLineXY = new TLine();
  fLineZY = new TLine();

  double  y1(6000.), y2(-1500);

  double x0    = fCTSeed->_track.FitParams.A0;
  double dxdy  = -fCTSeed->_track.FitParams.A1;

  double x1 = x0+dxdy*y1;
  double x2 = x0+dxdy*y2;

  fLineXY->SetX1(x1); 
  fLineXY->SetY1(y1); 
  fLineXY->SetX2(x2); 
  fLineXY->SetY2(y2); 
  
  double z0    = fCTSeed->_track.FitParams.B0;
  double dzdy  = -fCTSeed->_track.FitParams.B1;

  double z1 = z0+dzdy*y1;
  double z2 = z0+dzdy*y2;

  fLineZY->SetX1(z1); 
  fLineZY->SetY1(y1); 
  fLineZY->SetX2(z2); 
  fLineZY->SetY2(y2); 
    
  fLineXY->SetLineColor(kBlue+1);
  fLineZY->SetLineColor(kBlue+1);
}

//-----------------------------------------------------------------------------
TEvdCosmicTrack::~TEvdCosmicTrack() {
  delete fLineXY;
  delete fLineZY;

  if (fListOfHits) delete fListOfHits;
}

//-----------------------------------------------------------------------------
void TEvdCosmicTrack::Paint(Option_t* Option) {
  const char oname[] = "TEvdCosmicTrack::Paint";

  int view = TVisManager::Instance()->GetCurrentView()->Type();

  if      (view == TStnVisManager::kXY ) PaintXY (Option);
  else if (view == TStnVisManager::kRZ ) PaintRZ (Option);
  else if (view == TStnVisManager::kVST) PaintVST(Option);
  else if (view == TStnVisManager::kVRZ) PaintVRZ(Option);
  else if (view == TStnVisManager::kCal) {
//-----------------------------------------------------------------------------
// calorimeter-specific view: do not draw tracks but .. .later
//-----------------------------------------------------------------------------
  }
  else {
    printf("[%s] >>> ERROR: unknown view: %i, DO NOTHING\n",oname,view);
  }

  gPad->Modified();
}

//-----------------------------------------------------------------------------
// to display the reconstructed track in XY, use its parameters in the middle 
// of the tracker, at s=0
// display first segment 
//-----------------------------------------------------------------------------
void TEvdCosmicTrack::PaintXY(Option_t* Option) {
  fLineXY->Paint(Option);
}

//-----------------------------------------------------------------------------
void TEvdCosmicTrack::PaintRZ(Option_t* Option) {

  double y1(6000.), y2(-1500.);

  double rx1,rx2,ry1,ry2;
  gPad->GetRange(rx1,ry1,rx2,ry2);
                  
  double zoff = (rx1+rx2)/2.;
  
  double z0    =  fCTSeed->_track.FitParams.B0 - zoff;
  double dzdy  = -fCTSeed->_track.FitParams.B1;

  double z1 = z0+dzdy*y1;
  double z2 = z0+dzdy*y2;

  fLineZY->SetX1(z1); 
  fLineZY->SetY1(y1); 
  fLineZY->SetX2(z2); 
  fLineZY->SetY2(y2); 
    
  fLineZY->Paint(Option);
}

//-----------------------------------------------------------------------------
void TEvdCosmicTrack::PaintVST(Option_t* Option) {
  fLineZY->Paint(Option);
}

//-----------------------------------------------------------------------------
void TEvdCosmicTrack::PaintVRZ(Option_t* Option) {

  //TStnVisManager* vm = TStnVisManager::Instance();
  //  TGeoCombiTrans* gt = vm->GetCurrentView()->GetCombiTrans();
    
  // double posm[3], posl[3], dirm[3], dirl[3];
  // posm[0] = fCTSeed->_track.FitParams.A0;
  // posm[1] = 0.;
  // posm[2] = fCTSeed->_track.FitParams.B0;
  // gt->MasterToLocalVect(posm, posl);
  
  // dirm[0] = fCTSeed->_track.FitParams.A1;
  // dirm[1] = -1.;
  // dirm[2] = fCTSeed->_track.FitParams.B1;
  // gt->MasterToLocalVect(dirm, dirl);
 
  // double y1(-1000.), y2(1000.);

  // double dydz_l = dirl[1]/dirl[2];
  // double z1 = (y1-posl[1])/dydz_l + posl[2];
  // double z2 = (y2-posl[1])/dydz_l + posl[2];
  
  // TLine ln;
  // ln.SetLineColor(kBlue+1);
  // ln.PaintLine(z1,y1,z2,y2);
}

//-----------------------------------------------------------------------------
void TEvdCosmicTrack::PaintCal(Option_t* Option) {
}

//_____________________________________________________________________________
Int_t TEvdCosmicTrack::DistancetoPrimitive(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdCosmicTrack::DistancetoPrimitiveXY(Int_t px, Int_t py) {

  int dist(9999);

  // static TVector3 global;

  // global.SetXYZ(gPad->AbsPixeltoX(px),gPad->AbsPixeltoY(py),0);

  // double dx = global.X()-fEllipse->GetX1();
  // double dy = global.Y()-fEllipse->GetY1();

  // double dr = sqrt(dx*dx+dy*dy)-fEllipse->GetR1();

  // dist = gPad->XtoAbsPixel(global.x()+dr)-px;

  return abs(dist);
}

//_____________________________________________________________________________
Int_t TEvdCosmicTrack::DistancetoPrimitiveRZ(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdCosmicTrack::DistancetoPrimitiveVST(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdCosmicTrack::DistancetoPrimitiveVRZ(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdCosmicTrack::DistancetoPrimitiveCal(Int_t px, Int_t py) {
  return 9999;
}


//-----------------------------------------------------------------------------
void TEvdCosmicTrack::Clear(Option_t* Opt) {
  //  SetLineColor(1);
}


//-----------------------------------------------------------------------------
void TEvdCosmicTrack::PrintMe() const {
  printf("WARNING: TEvdCosmicTrack::PrintMe is yet to be implemented\n");
  
  // TStnVisManager* vm = TStnVisManager::Instance();
  // TVisNode* vn = vm->FindNode("TrkVisNode");
  // vn->NodePrint(fCTSeed,"CosmicTrackSeed");
}
//-----------------------------------------------------------------------------
void TEvdCosmicTrack::Print(Option_t* Opt) const {

  TStnVisManager* vm = TStnVisManager::Instance();

  TVisNode* vn = vm->FindNode("TrkVisNode");

  vn->NodePrint(fCTSeed,"CosmicTrackSeed");
}

}
