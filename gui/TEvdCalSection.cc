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

#include "Stntuple/gui/TEvdCluster.hh"
#include "Stntuple/gui/TEvdCrystal.hh"
#include "Stntuple/gui/TEvdCalSection.hh"
#include "Stntuple/gui/TStnVisManager.hh"
#include "Stntuple/gui/TCalView.hh"

//#include "CalorimeterGeom/inc/VaneCalorimeter.hh"
#include "Offline/CalorimeterGeom/inc/Crystal.hh"
#include "Offline/CalorimeterGeom/inc/Disk.hh"
#include "Offline/CalorimeterGeom/inc/DiskCalorimeter.hh"
#include "Offline/CalorimeterGeom/inc/Calorimeter.hh"

ClassImp(TEvdCalSection)

//_____________________________________________________________________________
TEvdCalSection::TEvdCalSection(const mu2e::Disk* Disk, int SectionID): TObject() {

  fDisk           = Disk;
  fSectionID      = SectionID;

  art::ServiceHandle<mu2e::GeometryService> geom;
//-----------------------------------------------------------------------------
// draw vanes or disks
//-----------------------------------------------------------------------------
  double rmin, rmax;

  if (geom->hasElement<mu2e::DiskCalorimeter>()) {

    rmin = fDisk->diskInfo().innerEnvelopeR();

    fEllipse[0] = new TEllipse(0.,0.,rmin,rmin,0.,360.,0);
    fEllipse[0]->SetLineColor(kGreen);
    fEllipse[0]->SetFillStyle(0);

    rmax = fDisk->diskInfo().outerEnvelopeR();
    
    fEllipse[1] = new TEllipse(0.,0.,rmax,rmax,0.,360.,0);
    fEllipse[1]->SetLineColor(kGreen);
    fEllipse[1]->SetFillStyle(0);
  }
}

//_____________________________________________________________________________
TEvdCalSection::~TEvdCalSection() {
  delete fEllipse[0];
  delete fEllipse[1];
}

//-----------------------------------------------------------------------------
void TEvdCalSection::Paint(Option_t* option) {
  // paints one disk (.. or vane, in the past), i.e. section

  //  char  v[100];
  //  int   iv;
				// parse option list
  int view = TVisManager::Instance()->GetCurrentView()->Type();

  if      (view == TStnVisManager::kXY ) PaintXY (option);
  else if (view == TStnVisManager::kCal) {
    int index = TVisManager::Instance()->GetCurrentView()->Index();
    if (index == fSectionID) {
      PaintCal(option);
    }
  }
  else {
    // what is the default?
    //    Warning("Paint",Form("Unknown option %s",option));
  }

  gPad->Modified();
}


//_____________________________________________________________________________
int TEvdCalSection::InitEvent() {
  printf(" ERROR: dummy TEvdCalSection::InitEvent() was called.\n");
  return 0;
}

//_____________________________________________________________________________
void TEvdCalSection::PaintXY(Option_t* Option) {
//-----------------------------------------------------------------------------
// draw vanes or disks
//-----------------------------------------------------------------------------
//  int nv(0);

  art::ServiceHandle<mu2e::GeometryService> geom;

//   if (geom->hasElement<mu2e::VaneCalorimeter>()) {
// //     mu2e::GeomHandle<mu2e::VaneCalorimeter> vc;
// //     nv = vc->nVane();

// //     for (int iv=0; iv<nv; iv++) {
// //       box->Paint(Option);
// //     }
//   }
//   else 
    if(geom->hasElement<mu2e::DiskCalorimeter>()) {
    fEllipse[0]->Paint(Option);
    fEllipse[1]->Paint(Option);
  }
}

//_____________________________________________________________________________
void TEvdCalSection::PaintCal(Option_t* Option) {
  // draw calorimeter

  art::ServiceHandle<mu2e::GeometryService> geom;
  mu2e::GeomHandle<mu2e::DiskCalorimeter> dc;

  //  TVisManager* vm = TVisManager::Instance();
  
  fEllipse[0]->Paint(Option);
  fEllipse[1]->Paint(Option);
//-----------------------------------------------------------------------------
// draw the calorimeter itself
//-----------------------------------------------------------------------------
//  int ncr = NCrystals();
//   TEvdCrystal* cr; 

//   for (int ic=0; ic<ncr; ic++) {
//     cr = fListOfCrystals->At(icr); 
//     cr->PaintCal(Option);
//   }
}


//_____________________________________________________________________________
void TEvdCalSection::PaintRZ(Option_t* option) {
  // draw calorimeter
}

//_____________________________________________________________________________
Int_t TEvdCalSection::DistancetoPrimitive(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdCalSection::DistancetoPrimitiveXY(Int_t px, Int_t py) {

  Int_t dist = 9999;

  static TVector3 global;
//   static TVector3 local;

  //  Double_t    dx1, dx2, dy1, dy2, dx_min, dy_min, dr;

  global.SetXYZ(gPad->AbsPixeltoX(px),gPad->AbsPixeltoY(py),0);

  return dist;
}

//_____________________________________________________________________________
Int_t TEvdCalSection::DistancetoPrimitiveRZ(Int_t px, Int_t py) {
  return 9999;
}

