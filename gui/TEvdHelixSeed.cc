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

#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"

#include "Offline/TrackerGeom/inc/Tracker.hh"

#include "Offline/RecoDataProducts/inc/TrkStrawHitSeed.hh"
#include "Offline/RecoDataProducts/inc/HelixSeed.hh"

#include "Stntuple/print/TAnaDump.hh"

#include "Stntuple/gui/TEvdHelixVisNode.hh"
#include "Stntuple/gui/TEvdHelixSeed.hh"
#include "Stntuple/gui/TStnVisManager.hh"
#include "Stntuple/gui/TEvdTrkStrawHit.hh"
#include "Stntuple/base/TObjHandle.hh"

#include "CLHEP/Vector/ThreeVector.h"

ClassImp(stntuple::TEvdHelixSeed)

namespace stntuple {

//-----------------------------------------------------------------------------
TEvdHelixSeed::TEvdHelixSeed(): TObject() {
  fListOfHits = NULL;
  fEllipse = new TEllipse();
}

//-----------------------------------------------------------------------------
// pointer to track is just cached
//-----------------------------------------------------------------------------
  TEvdHelixSeed::TEvdHelixSeed(int Number, const mu2e::HelixSeed* HSeed, TStnVisNode* VisNode): TObject() {
  fNumber    = Number;
  fHelixSeed = HSeed;
  fVisNode   = VisNode;

  fListOfHits = new TObjArray();

  fEllipse = new TEllipse();

  // fEllipse->SetFillStyle(3001);		// make it transparent
    
  fEllipse->SetLineColor(kGreen+4);
  fEllipse->SetLineStyle(4);
  fEllipse->SetLineWidth(3);
}

//-----------------------------------------------------------------------------
TEvdHelixSeed::~TEvdHelixSeed() {
  delete fEllipse;

  if (fListOfHits) {
    fListOfHits->Delete();
    delete fListOfHits;
  }
}

//-----------------------------------------------------------------------------
void TEvdHelixSeed::Paint(Option_t* Option) {
  const char oname[] = "TEvdHelixSeed::Paint";

  int view = TVisManager::Instance()->GetCurrentView()->Type();

  if      (view == TStnVisManager::kXY ) PaintXY (Option);
  else if (view == TStnVisManager::kRZ ) PaintRZ (Option);
  else if (view == TStnVisManager::kCal) {
//-----------------------------------------------------------------------------
// calorimeter-specific view: do not draw tracks
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
//-----------------------------------------------------------------------------
void TEvdHelixSeed::PaintXY(Option_t* Option) {

  double r, x0, y0;

  const mu2e::RobustHelix* hel = &fHelixSeed->helix();

  r    = hel->radius();
    
  x0   = hel->centerx();
  y0   = hel->centery();

  // printf("[MuHitDispla::printHelixParams] d0 = %5.3f r = %5.3f phi0 = %5.3f x0 = %5.3f y0 = %5.3f\n",
  // 	 d0, r, phi0, x0, y0);
   
  fEllipse->SetR1(r);
  fEllipse->SetR2(r);
  fEllipse->SetX1(x0);
  fEllipse->SetY1(y0);
  fEllipse->SetPhimin(0);
  fEllipse->SetPhimax(2*M_PI*180);
  fEllipse->SetTheta(0);

  fEllipse->SetFillStyle(0);
  fEllipse->SetFillColor(0);
  fEllipse->SetLineColor(kGreen+4);
  fEllipse->PaintEllipse(x0,y0,r,r,0,2*M_PI*180,0);
  //fEllipse->Paint();
}

//-----------------------------------------------------------------------------
void TEvdHelixSeed::PaintRZ(Option_t* Option) {

  double            zwire[2], rdrift, zt[4], rt[4], zw, rw;
  //  CLHEP::Hep3Vector tdir;
  //  HepPoint          tpos;
  TPolyLine         pline;
  int               nplanes, /*npanels,*/ nl;

  mu2e::GeomHandle<mu2e::Tracker> handle;

  const mu2e::Tracker* tracker = handle.get();

  const mu2e::Straw  *s, *straw[2];		// first straw
  
  nplanes = tracker->nPlanes();
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
  const mu2e::RobustHelix* hel = &fHelixSeed->helix();

  const mu2e::TrkStrawHitSeed  *hit;

  int nhits = NHits();
  for (int i=0; i<nhits; i++) {
    hit    = Hit(i)->TrkStrawHitSeed();
    rdrift = hit->driftRadius();

    mu2e::StrawId sid = hit->strawId();

    const mu2e::Straw* hstraw = &tracker->straw(sid);

    zw     = hstraw->getMidPoint().z();
    rw     = hstraw->getMidPoint().perp();

    fEllipse->SetX1(zw);
    fEllipse->SetY1(rw);
    fEllipse->SetR1(0);
    fEllipse->SetR2(rdrift);
    fEllipse->SetFillStyle(3003);

    // if (hit->isActive()) fEllipse->SetFillColor(kRed);
    // else                 fEllipse->SetFillColor(kBlue+3);

    fEllipse->SetFillColor(kGreen+4);
    fEllipse->PaintEllipse(zw,rw,rdrift,0,0,2*M_PI*180,0);
  }
//-----------------------------------------------------------------------------
// now draw the trajectory in Z-R(local) space - device is a station
//-----------------------------------------------------------------------------
//  HelixTraj trkHel(fHelixSeed->helix().params(),fHelixSeed->helix().covariance());

  for (int iplane=0; iplane<nplanes; iplane++) {
    const mu2e::Plane* plane = &tracker->getPlane(iplane);
//-----------------------------------------------------------------------------
// a plane is made of 2 'faces' or 6 panels, panels 0 and 1 on different faces
//-----------------------------------------------------------------------------
//    npanels = plane->nPanels();
//-----------------------------------------------------------------------------
// 3 panels in the same plane have the same Z and do not overlap - 
// no need to duplicate; panels 0 and 1 are at different Z
//-----------------------------------------------------------------------------
    for (int iface=0; iface<2; iface++) {
      const mu2e::Panel* panel = &plane->getPanel(iface);
      nl       = panel->nLayers();
					// deal with the compiler warnings
      zwire[0] = -1.e6;
      zwire[1] = -1.e6;

      for (int il=0; il<nl; il++) {
//-----------------------------------------------------------------------------
// assume all wires in a layer have the same Z, extrapolate track to the layer
//-----------------------------------------------------------------------------
	straw[il] = &panel->getStraw(il);
	zwire[il] = straw[il]->getMidPoint().z();
      }
					// order locally in Z
      if (zwire[0] > zwire[1]) {
	zt[1] = zwire[1];
	zt[2] = zwire[0];
	//	flip  = 1.;
      }
      else {
	zt[1] = zwire[0];
	zt[2] = zwire[1];
	//	flip  = -1.;
      }
					// z-step between the layers, want two more points
      zt[0] = zt[1]-3.;
      zt[3] = zt[2]+3.;

      const CLHEP::Hep3Vector* wd;
      double r;
      
      for (int ipoint=0; ipoint<4; ipoint++) {
//-----------------------------------------------------------------------------
// estimate flen using helix
//-----------------------------------------------------------------------------
	// flen   = trkHel.zFlight(zt[ipoint]);
	// fKrep->traj().getInfo(flen,tpos,tdir);
	XYZVectorF tpos = hel->position(zt[ipoint]);
//-----------------------------------------------------------------------------
// for each face, loop over 3 panels in it
//-----------------------------------------------------------------------------
	rt[ipoint] = -1.e6;
	for (int ipp=0; ipp<3; ipp++) {
	  const mu2e::Panel* pp = &plane->getPanel(2*ipp+iface);
	  s  = &pp->getStraw(0);
	  wd = &s->getDirection();
	  r  = (tpos.x()*wd->y()-tpos.y()*wd->x()); // *flip;
	  if (r > rt[ipoint]) rt[ipoint] = r;
	}
      }
      pline.SetLineWidth(2);
      pline.PaintPolyLine(4,zt,rt);
    }
  }

}

//-----------------------------------------------------------------------------
void TEvdHelixSeed::PaintCal(Option_t* Option) {
}

//_____________________________________________________________________________
Int_t TEvdHelixSeed::DistancetoPrimitive(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdHelixSeed::DistancetoPrimitiveXY(Int_t px, Int_t py) {

  Int_t dist = 9999;

  static TVector3 global;
//   static TVector3 local;

//   Double_t    dx1, dx2, dy1, dy2, dx_min, dy_min, dr;

  global.SetXYZ(gPad->AbsPixeltoX(px),gPad->AbsPixeltoY(py),0);

  double dx = global.X()-fEllipse->GetX1();
  double dy = global.Y()-fEllipse->GetY1();

  double dr = sqrt(dx*dx+dy*dy)-fEllipse->GetR1();

  dist = gPad->XtoAbsPixel(global.x()+dr)-px;

  return abs(dist);
}

//_____________________________________________________________________________
Int_t TEvdHelixSeed::DistancetoPrimitiveRZ(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdHelixSeed::DistancetoPrimitiveCal(Int_t px, Int_t py) {
  return 9999;
}


//-----------------------------------------------------------------------------
void TEvdHelixSeed::Clear(Option_t* Opt) {
  //  SetLineColor(1);
}


//-----------------------------------------------------------------------------
void TEvdHelixSeed::Print(Option_t* Option) const {

  TAnaDump* ad = TAnaDump::Instance();

  TEvdHelixVisNode* vn = (TEvdHelixVisNode*) fVisNode;

  TString opt = Option;
  opt.ToLower();

  if (opt == "") ad->printHelixSeed(fHelixSeed,vn->ShCollTag().encode().data(),vn->SdmcCollTag().encode().data(), "");
  else           ad->printHelixSeed(fHelixSeed,vn->ShCollTag().encode().data(),vn->SdmcCollTag().encode().data(),opt);
}
}
