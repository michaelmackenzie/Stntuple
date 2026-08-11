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
// clang-format off
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

#include "Offline/RecoDataProducts/inc/KalSegment.hh"
#include "Offline/RecoDataProducts/inc/KalSeed.hh"

#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"

#include "Offline/TrackerGeom/inc/Tracker.hh"

#include "Stntuple/gui/TEvdTrack.hh"
#include "Stntuple/gui/TStnVisManager.hh"
#include "Stntuple/gui/TStnGeoManager.hh"
#include "CLHEP/Vector/ThreeVector.h"

ClassImp(stntuple::TEvdTrack)

namespace stntuple {

//-----------------------------------------------------------------------------
TEvdTrack::TEvdTrack(): TObject() {
  fKSeed      = nullptr;
  fListOfHits = nullptr;
  fEllipse    = nullptr;
  fLineXY     = nullptr;
  fLineZY     = nullptr;
}

//-----------------------------------------------------------------------------
// pointer to track is just cached
//-----------------------------------------------------------------------------
  TEvdTrack::TEvdTrack(int Number, const mu2e::KalSeed* KSeed): TObject() {
  fNumber  = Number;
  fKSeed   = KSeed;

  fListOfHits = new TObjArray();
  fListOfHits->SetOwner(kTRUE);

  fEllipse = new TEllipse();

  // fEllipse->SetFillStyle(3001);		// make it transparent
    
  fEllipse->SetLineColor(kBlue-7);

  auto gm = TStnGeoManager::Instance();

  if (gm->BField()) {

    const mu2e::KalSegment* kseg = &fKSeed->segments().at(0);

    KinKal::LoopHelix helx  = kseg->loopHelix();

    //  KinKal::CentralHelix helx  = kseg->centralHelix();

    double r    = fabs(helx.rad());
    double x0   = helx.cx();
    double y0   = helx.cy();

    // if (helx.charge() < 0) {
    //   x0 = -helx.cy();
    //   y0 = -helx.cx();
    // }
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
    fEllipse->SetLineColor(2);
  }
  else {
    KinKal::KinematicLine kline = fKSeed->segments().back().kinematicLine();
    KinKal::VEC3 pos = kline.pos0();
    KinKal::VEC3 dir = kline.direction();
    
    double dydx = dir.y()/dir.x();

    double y1(1000.), y2(-1000.);

    double x1 = (y1-pos.y())/dydx + pos.x();
    double x2 = (y2-pos.y())/dydx + pos.x();
                                        // global reference frame
    fLineXY   = new TLine(x1,y1,x2,y2);
    // fLineXY->SetLineColor(color);

    double dydz = dir.y()/dir.z();
    double z1 = (y1-pos.y())/dydz + pos.z();
    double z2 = (y2-pos.y())/dydz + pos.z();
                                        // in the global reference frame
    fLineZY   = new TLine(z1,y1,z2,y2);
  }

  
}

//-----------------------------------------------------------------------------
TEvdTrack::~TEvdTrack() {
  delete fEllipse;

  if (fListOfHits) {
    delete fListOfHits;
  }
}

//-----------------------------------------------------------------------------
void TEvdTrack::Paint(Option_t* Option) {
  const char oname[] = "TEvdTrack::Paint";

  int view = TVisManager::Instance()->GetCurrentView()->Type();

  if      (view == TStnVisManager::kXY ) PaintXY (Option);
  else if (view == TStnVisManager::kRZ ) PaintRZ (Option);
  else if (view == TStnVisManager::kVRZ) PaintVRZ(Option);
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
// display first segment 
//-----------------------------------------------------------------------------
void TEvdTrack::PaintXY(Option_t* Option) {

  TStnGeoManager* gm = TStnGeoManager::Instance();
  if (gm->BField()) {
    //    fEllipse->PaintEllipse(x0,y0,r,r,0,2*M_PI*180,0);
    fEllipse->Paint();
  }
  else {
//-----------------------------------------------------------------------------
// no field, cosmics
//-----------------------------------------------------------------------------
    fLineXY->Paint();
  }
}

//-----------------------------------------------------------------------------
void TEvdTrack::PaintVRZ(Option_t* Option) {
  
  TStnVisManager*    vm    = TStnVisManager::Instance();
  const mu2e::Panel* panel = (const mu2e::Panel*) vm->GetCurrentView()->GetMother();
  const mu2e::HepTransform&   ht = panel->dsToPanel();

  const CLHEP::Hep3Vector& o1 = panel->origin();
  ROOT::Math::XYZVectorF   o2(o1.x(),o1.y(),o1.z()); 
  KinKal::KinematicLine   kline = fKSeed-> nearestSegment(o2)->kinematicLine();
  ROOT::Math::XYZVector   pos   = kline.pos0();
  ROOT::Math::XYZVector   dir   = kline.direction();
  // rotate to the view frame

  double dirl[3];

  CLHEP::Hep3Vector pos_m(pos.x(),pos.y(),pos.z());
  CLHEP::Hep3Vector pos_l = ht*pos_m;
 
  CLHEP::Hep3Vector dir_m(dir.x(),dir.y(),dir.z());
  CLHEP::Hep3Vector dir_l = ht.rotation()*dir_m;
  dirl[0] = dir_l.x();
  dirl[1] = dir_l.y();
  dirl[2] = dir_l.z();
  
  //  std::cout << "panel:" << panel->id().plane() << ":" << panel->id().panel() << " pos_l:" << pos_l << " dir_l:" << dir_l << std::endl;
 
  double y1(200.), y2(-200.);           // in the local coordinate system

  double dydz_l = dirl[1]/dirl[2];
  double z1     = (y1-pos_l[1])/dydz_l + pos_l[2];
  double z2     = (y2-pos_l[1])/dydz_l + pos_l[2];

  // std::cout << "y1,y2,z1,z2:" << std::setw(13) << y1 << std::setw(13) << y2 << std::setw(13) << z1 << std::setw(13) << z2 << std::endl;
  TLine ln;
  ln.SetLineColor(kRed+1);
  ln.PaintLine(z1,y1,z2,y2);
}

//-----------------------------------------------------------------------------
void TEvdTrack::PaintRZ(Option_t* Option) {
////   double            flen, zwire[2], ds, rdrift, zt[4], rt[4], zw, rw;
////   CLHEP::Hep3Vector tdir;
////   HepPoint          tpos;
////   TPolyLine         pline;
////   int               nplanes, /*npanels,*/ nl;
//// 
////   mu2e::GeomHandle<mu2e::Tracker> handle;
//// 
////   const mu2e::Tracker* tracker = handle.get();
//// 
////   const mu2e::Straw  *hstraw, *s, *straw[2];		// first straw
////   
////   nplanes = tracker->nPlanes();
//// //-----------------------------------------------------------------------------
//// // first display track hits - active and not 
//// //-----------------------------------------------------------------------------
////   const mu2e::TrkStrawHit  *hit;
////   //   const TrkHitVector*       hits = &fKrep->hitVector();
//// 
////   int nhits = NHits();
////   for (int i=0; i<nhits; i++) {
////     hit = Hit(i)->TrkStrawHit();
////     rdrift = hit->driftRadius();
//// 
////     hstraw = &hit->straw();
////     zw     = hstraw->getMidPoint().z();
////     rw     = hstraw->getMidPoint().perp();
//// 
////     fEllipse->SetX1(zw);
////     fEllipse->SetY1(rw);
////     fEllipse->SetR1(0);
////     fEllipse->SetR2(rdrift);
////     fEllipse->SetFillStyle(3003);
//// 
////     if (hit->isActive()) fEllipse->SetFillColor(kRed);
////     else                 fEllipse->SetFillColor(kBlue+3);
//// 
////     fEllipse->PaintEllipse(zw,rw,rdrift,0,0,2*M_PI*180,0);
////   }
//// 
////   for (int iplane=0; iplane<nplanes; iplane++) {
////     const mu2e::Plane* plane = &tracker->getPlane(iplane);
//// //-----------------------------------------------------------------------------
//// // a plane is made of 2 'faces' or 6 panels, panels 0 and 1 on different faces
//// //-----------------------------------------------------------------------------
//// //    npanels = plane->nPanels();
//// //-----------------------------------------------------------------------------
//// // 3 panels in the same plane have the same Z and do not overlap - 
//// // no need to duplicate; panels 0 and 1 are at different Z
//// //-----------------------------------------------------------------------------
////     for (int iface=0; iface<2; iface++) {
////       const mu2e::Panel* panel = &plane->getPanel(iface);
////       nl       = panel->nLayers();
//// 					// deal with the compiler warnings
////       zwire[0] = -1.e6;
////       zwire[1] = -1.e6;
//// 
////       for (int il=0; il<nl; il++) {
//// //-----------------------------------------------------------------------------
//// // assume all wires in a layer have the same Z, extrapolate track to the layer
//// //-----------------------------------------------------------------------------
//// 	straw[il] = &panel->getStraw(il);
//// 	zwire[il] = straw[il]->getMidPoint().z();
////       }
//// 					// order locally in Z
////       if (zwire[0] > zwire[1]) {
//// 	zt[1] = zwire[1];
//// 	zt[2] = zwire[0];
//// 	//	flip  = 1.;
////       }
////       else {
//// 	zt[1] = zwire[0];
//// 	zt[2] = zwire[1];
//// 	//	flip  = -1.;
////       }
//// 					// z-step between the layers, want two more points
////       zt[0] = zt[1]-3.;
////       zt[3] = zt[2]+3.;
//// 
////       const CLHEP::Hep3Vector* wd;
////       double r;
////       
//// //-----------------------------------------------------------------------------
//// // pick the right track segment to display
//// //-----------------------------------------------------------------------------
////       // HelixTraj trkHel(fKrep->helix(0).params(),fKrep->helix(0).covariance());
//// 
//// //-----------------------------------------------------------------------------
//// // find segments corresponding to entry and exit points
//// //-----------------------------------------------------------------------------
////     const mu2e::KalSegment *kseg(nullptr), *kseg_exit(nullptr);
//// 
////     double min_ds1(1.e6), min_ds2(1.e6);
//// 
////     for(auto const& ks : fKSeed->segments() ) {
//// 
////       KinKal::CentralHelix helx  = ks.centralHelix();
////       double z0 = helx.z0();
////       
////       double smin = ks.timeToFlt(ks.tmin());
////       double smax = ks.timeToFlt(ks.tmax());
//// 
////       double ds1 = fabs(sent-(smin+smax)/2);
////       if (ds1 < min_ds1) {
//// 	kseg    = &ks;
//// 	min_ds1 = ds1;
////       }
//// 
////       double ds2 = fabs(sexit-(smin+smax)/2);
////       if (ds2 < min_ds2) {
//// 	kseg_exit = &ks;
//// 	min_ds2   = ds2;
////       }
//// 
//// 
//// 
//// 
//// 
////       for (int ipoint=0; ipoint<4; ipoint++) {
//// //-----------------------------------------------------------------------------
//// // estimate flen using helix
//// //-----------------------------------------------------------------------------
//// 	flen   = trkHel.zFlight(zt[ipoint]);
//// 	fKrep->traj().getInfo(flen,tpos,tdir);
//// //-----------------------------------------------------------------------------
//// // try to extrapolate helix to a given Z a bit more accurately 
//// //-----------------------------------------------------------------------------
//// 	ds     = (zt[ipoint]-tpos.z())/tdir.z();
//// 	fKrep->traj().getInfo(flen+ds,tpos,tdir);
//// //-----------------------------------------------------------------------------
//// // for each face, loop over 3 panels in it
//// //-----------------------------------------------------------------------------
//// 	rt[ipoint] = -1.e6;
//// 	for (int ipp=0; ipp<3; ipp++) {
//// 	  const mu2e::Panel* pp = &plane->getPanel(2*ipp+iface);
//// 	  s  = &pp->getStraw(0);
//// 	  wd = &s->getDirection();
//// 	  r  = (tpos.x()*wd->y()-tpos.y()*wd->x()); // *flip;
//// 	  if (r > rt[ipoint]) rt[ipoint] = r;
//// 	}
////       }
////       pline.SetLineWidth(2);
////       pline.PaintPolyLine(4,zt,rt);
////     }
////   }
//// 
}

//-----------------------------------------------------------------------------
void TEvdTrack::PaintCal(Option_t* Option) {
}

//_____________________________________________________________________________
Int_t TEvdTrack::DistancetoPrimitive(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdTrack::DistancetoPrimitiveXY(Int_t px, Int_t py) {

  int dist(9999);

  static TVector3 global;

  global.SetXYZ(gPad->AbsPixeltoX(px),gPad->AbsPixeltoY(py),0);

  double dx = global.X()-fEllipse->GetX1();
  double dy = global.Y()-fEllipse->GetY1();

  double dr = sqrt(dx*dx+dy*dy)-fEllipse->GetR1();

  dist = gPad->XtoAbsPixel(global.x()+dr)-px;

  return abs(dist);
}

//_____________________________________________________________________________
Int_t TEvdTrack::DistancetoPrimitiveRZ(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdTrack::DistancetoPrimitiveVRZ(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
Int_t TEvdTrack::DistancetoPrimitiveCal(Int_t px, Int_t py) {
  return 9999;
}


//-----------------------------------------------------------------------------
void TEvdTrack::Clear(Option_t* Opt) {
  //  SetLineColor(1);
}


//-----------------------------------------------------------------------------
void TEvdTrack::Print(Option_t* Opt) const {

  TStnVisManager* vm = TStnVisManager::Instance();

  TVisNode* vn = vm->FindNode("TrkVisNode");

  vn->NodePrint(fKSeed,"KalSeed");
}

}
