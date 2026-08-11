///////////////////////////////////////////////////////////////////////////////
// May 04 2022 P.Murat
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
#include "TDatabasePDG.h"


// #include "art/Framework/Principal/Handle.h"

                                        // probably will go away 
#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"

#include "Offline/TrackerGeom/inc/Tracker.hh"

#include "Offline/MCDataProducts/inc/SimParticle.hh"
#include "Offline/MCDataProducts/inc/StrawGasStep.hh"

#include "Stntuple/obj/TSimParticle.hh"

#include "Stntuple/gui/TEvdSimParticle.hh"
#include "Stntuple/gui/TStnGeoManager.hh"
#include "Stntuple/gui/TStnVisManager.hh"
// #include "Stntuple/gui/TEvdTrkStrawHit.hh"

#include "Stntuple/print/TAnaDump.hh"

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Geometry/Point3D.h"

ClassImp(stntuple::TEvdSimParticle)

namespace stntuple {

//-----------------------------------------------------------------------------
TEvdSimParticle::TEvdSimParticle(): TObject() {
  fListOfHits  = NULL;
  fEllipse     = nullptr;
  fLineXY      = nullptr;
  fLineTZ      = nullptr;
  fParticlePDG = nullptr;
  fS1          = nullptr;
  fS2          = nullptr;
  fSimp        = nullptr;
}

//-----------------------------------------------------------------------------
// pointer to track is just cached
// Step - at the trackerfront
//-----------------------------------------------------------------------------
  TEvdSimParticle::TEvdSimParticle(int                       Number, 
				   TSimParticle*             Simp  , 
				   const mu2e::StrawGasStep* S1    ,
				   const mu2e::StrawGasStep* S2    ): TObject() {
  fNumber = Number;
  fSimp   = Simp;
  fS1     = S1;
  fS2     = S2;

  TStnVisManager* vm = TStnVisManager::Instance();
  TStnGeoManager* gm = TStnGeoManager::Instance();

  TDatabasePDG* pdb = TDatabasePDG::Instance();

  fParticlePDG = pdb->GetParticle(fSimp->PDGCode());

  double p      = fS1->momentum().R  ();
  int    pdg_id = fSimp->PDGCode();

  int color(0);

  if      (pdg_id == 11) {
    if    (p       > 20  ) { color = kRed;      }
    else                   { color = kRed+2;    }
  }
  else if (pdg_id ==  -11) { color = kBlue;     } 
  else if (pdg_id ==   13) { color = kGreen+2;  } 
  else if (pdg_id ==  -13) { color = kGreen-2;  }
  else if (pdg_id ==   13) { color = kGreen+2;  } 
  else if (pdg_id ==  -13) { color = kGreen-2;  }
  else if (pdg_id == 2212) { color = kBlue+2;   }
  else if (pdg_id ==  211) { color = kOrange-3; } 
  else if (pdg_id == -211) { color = kOrange+7; }
  else                     { color = kBlack;    } 

  fListOfHits = new TObjArray();

  double r, phi0, x0, y0, xc, yc, pt, q;

  x0 = fS1->startPosition().x();	// in mm
  y0 = fS1->startPosition().y();

  if (gm->BField() != 0) {
					// rely on particle traveling along the Z axis
    pt = fS1->momentum().Rho();
    r  = pt/2.9979*10/gm->BField();	//    10^10/c, in mm

    phi0 = fS1->momentum().Phi();

    q = fParticlePDG->Charge()/3;	// returned is the charge in units of 1/3 
    
    xc   =  x0+r*q*sin(phi0);
    yc   =  y0-r*q*cos(phi0);

    // printf("[MuHitDispla::printHelixParams] d0 = %5.3f r = %5.3f phi0 = %5.3f x0 = %5.3f y0 = %5.3f\n",
    //       d0, r, phi0, x0, y0);
   
    fEllipse = new TEllipse();
    fEllipse->SetR1(r);
    fEllipse->SetR2(r);
    fEllipse->SetX1(xc);
    fEllipse->SetY1(yc);
    fEllipse->SetPhimin(0);
    fEllipse->SetPhimax(2*M_PI*180);
    fEllipse->SetTheta(0);

    fEllipse->SetFillStyle(0);
    fEllipse->SetFillColor(0);
    fEllipse->SetLineColor(color);

    // fEllipse->SetFillStyle(3001);		// make it transparent
    fLineXY = nullptr;
  }
  else {
//-----------------------------------------------------------------------------
// zero field - assume cosmics ... 
// also assume that the particle's trajectory is not horisontal
//-----------------------------------------------------------------------------
    double dydx = fS1->momentum().y()/fS1->momentum().x();

    double y1(1000.), y2(-1000.);

    double x1 = (y1-y0)/dydx + x0;
    double x2 = (y2-y0)/dydx + x0;
    
    fLineXY   = new TLine(x1,y1,x2,y2);
    fLineXY->SetLineColor(color);
    fEllipse  = nullptr;
  }
//-----------------------------------------------------------------------------
// TZ view, Z positons of StrawGasSteps - in the tracker system
//-----------------------------------------------------------------------------
  double z0    = fS1->startPosition().z();   // tracker->origin().z();
  double z1    = fS2->startPosition().z();   // tracker->origin().z();
  double t0    = fmod(fS1->time(),vm->MbTime());

  double m = fParticlePDG->Mass()*1e3;          // returned mass is in (GeV)
  //  double p     = fStep->momentum().mag();
  double vz    = fS1->momentum().z()/sqrt(p*p+m*m)*300.;  // vz in cm/ns

  double t1    = fmod(t0+(z1-z0)/vz,vm->MbTime());

  fLineTZ      = new TLine(z0,t0,z1,t1);
  fLineTZ->SetLineColor(color);
}

//-----------------------------------------------------------------------------
TEvdSimParticle::~TEvdSimParticle() {
  if (fEllipse) delete fEllipse;
  if (fLineXY ) delete fLineXY;

  delete fLineTZ;

  if (fListOfHits) {
    fListOfHits->Delete();
    delete fListOfHits;
  }
}

//-----------------------------------------------------------------------------
void TEvdSimParticle::Paint(Option_t* Option) {
  const char oname[] = "TEvdSimParticle::Paint";

  int view = TVisManager::Instance()->GetCurrentView()->Type();

  if      (view == TStnVisManager::kXY  ) PaintXY  (Option);
  else if (view == TStnVisManager::kRZ  ) PaintRZ  (Option);
  else if (view == TStnVisManager::kPhiZ) PaintPhiZ(Option);
  else if (view == TStnVisManager::kCal ) {
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
void TEvdSimParticle::PaintXY(Option_t* Option) {
  TStnGeoManager* gm = TStnGeoManager::Instance();
  if (gm->BField() != 0) fEllipse->Paint();
  else                   fLineXY->Paint();
}

//-----------------------------------------------------------------------------
// to display the reconstructed track in XY, use its parameters in the middle 
// of the tracker, at s=0
//-----------------------------------------------------------------------------
void TEvdSimParticle::PaintTZ(Option_t* Option) {
  fLineTZ->Paint();
}

//-----------------------------------------------------------------------------
// display the SimParticle in Phi:Z view
// empty for now
//-----------------------------------------------------------------------------
void TEvdSimParticle::PaintPhiZ(Option_t* Option) {
  // fLineTZ->Paint();
}

//-----------------------------------------------------------------------------
void TEvdSimParticle::PaintRZ(Option_t* Option) {

  //  double            flen, zwire[2], ds, rdrift, zt[4], rt[4], zw, rw;
  double            zwire[2], rdrift(0.), zt[4], rt[4], zw, rw;
  CLHEP::Hep3Vector tdir;
  // HepPoint          tpos;
  TPolyLine         pline;
  int               nplanes, /*npanels,*/ nl;

  mu2e::GeomHandle<mu2e::Tracker> handle;

  const mu2e::Tracker* tracker = handle.get();

  //  const mu2e::Straw  *hstraw, *s, *straw[2];		// first straw
  const mu2e::Straw  /**hstraw,*/ *straw[2];		// first straw
  
  nplanes = tracker->nPlanes();
//-----------------------------------------------------------------------------
// first display track hits - active and not 
//-----------------------------------------------------------------------------
  // const mu2e::TrkStrawHit  *hit;
  //   const TrkHitVector*       hits = &fKrep->hitVector();

  int nhits = 0; // NHits();
  for (int i=0; i<nhits; i++) {
    // hit = nullptr; // Hit(i)->TrkStrawHit();
    // rdrift = hit->driftRadius();

    // hstraw = &hit->straw();
    // zw     = hstraw->getMidPoint().z();
    // rw     = hstraw->getMidPoint().perp();

    fEllipse->SetX1(zw);
    fEllipse->SetY1(rw);
    fEllipse->SetR1(0);
    fEllipse->SetR2(rdrift);
    fEllipse->SetFillStyle(3003);

    // if (hit->isActive()) fEllipse->SetFillColor(kRed);
    // else                 fEllipse->SetFillColor(kBlue+3);

    fEllipse->PaintEllipse(zw,rw,rdrift,0,0,2*M_PI*180,0);
  }
//-----------------------------------------------------------------------------
// now draw the trajectory in Z-R(local) space - device is a station
//-----------------------------------------------------------------------------
//  HelixTraj trkHel(fKrep->helix(0).params(),fKrep->helix(0).covariance());

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

      // const CLHEP::Hep3Vector* wd;
      // double r;
      
//       for (int ipoint=0; ipoint<4; ipoint++) {
// //-----------------------------------------------------------------------------
// // estimate flen using helix
// //-----------------------------------------------------------------------------
// 	flen   = trkHel.zFlight(zt[ipoint]);
// 	fKrep->traj().getInfo(flen,tpos,tdir);
// //-----------------------------------------------------------------------------
// // try to extrapolate helix to a given Z a bit more accurately 
// //-----------------------------------------------------------------------------
// 	ds     = (zt[ipoint]-tpos.z())/tdir.z();
// 	fKrep->traj().getInfo(flen+ds,tpos,tdir);
// //-----------------------------------------------------------------------------
// // for each face, loop over 3 panels in it
// //-----------------------------------------------------------------------------
// 	rt[ipoint] = -1.e6;
// 	for (int ipp=0; ipp<3; ipp++) {
// 	  const mu2e::Panel* pp = &plane->getPanel(2*ipp+iface);
// 	  s  = &pp->getStraw(0);
// 	  wd = &s->getDirection();
// 	  r  = (tpos.x()*wd->y()-tpos.y()*wd->x()); // *flip;
// 	  if (r > rt[ipoint]) rt[ipoint] = r;
// 	}
//       }
      pline.SetLineWidth(2);
      pline.PaintPolyLine(4,zt,rt);
    }
  }

}

//_____________________________________________________________________________
int TEvdSimParticle::DistancetoPrimitive(Int_t px, Int_t py) {
  return 9999;
}

//_____________________________________________________________________________
int TEvdSimParticle::DistancetoPrimitiveXY(Int_t px, Int_t py) {

  static TVector3 global(gPad->AbsPixeltoX(px),gPad->AbsPixeltoY(py),0);

  int    dist(9999), dpy(10);

  TStnGeoManager* gm = TStnGeoManager::Instance();

  if (gm->BField() != 0) {


    double x1 = gPad->PixeltoX(px);
    double y1 = gPad->PixeltoY(py);

    double dx = x1-fEllipse->GetX1();
    double dy = y1-fEllipse->GetY1();

    double dr = sqrt(dx*dx+dy*dy)-fEllipse->GetR1();
    
    dist = gPad->XtoAbsPixel(global.x()+dr)-px;
  }
  else {
    TVector3 g2(gPad->AbsPixeltoX(px),gPad->AbsPixeltoY(py+dpy),0);

    double dy    = g2.y()-global.y();

    double x0    = fS1->position().x();
    double y0    = fS1->position().y();

    double nux   = -fS1->momentum().y()/fS1->momentum().R();
    double nuy   =  fS1->momentum().x()/fS1->momentum().R();

    double dt    = (global.x()-x0)*nux+(global.y()-y0)*nuy;

    dist         = (dt/dy)*dpy; // in pixels
  }

  return abs(dist);
}

//-----------------------------------------------------------------------------
Int_t TEvdSimParticle::DistancetoPrimitiveRZ(Int_t px, Int_t py) {
  return 9999;
}

//-----------------------------------------------------------------------------
// X represents Z on this view
//-----------------------------------------------------------------------------
Int_t TEvdSimParticle::DistancetoPrimitiveTZ(Int_t px, Int_t py) {

  static TVector3 global, g2;

  int dpy (10);

  TStnVisManager* vm = TStnVisManager::Instance();

  global.SetXYZ(gPad->AbsPixeltoX(px),gPad->AbsPixeltoY(py),0);
  g2.SetXYZ    (gPad->AbsPixeltoX(px),gPad->AbsPixeltoY(py+dpy),0);

  double  dy   = g2.y()-global.y();

  double z0    = fS1->position().z();
  double t0    = fmod(fS1->time(),vm->MbTime());

  double m     = fParticlePDG->Mass()*1e3;     // returned mass is in (GeV)
  double p     = fS1->momentum().R();
  double vz    = fS1->momentum().z()/sqrt(p*p+m*m)*300.;  // vz in cm/ns

  double tz    = fmod(t0+(global.x()-z0)/vz,vm->MbTime());

  double dt    = global.y()-tz;

  int dist     = (dt/dy)*dpy; // in pixels
  return abs(dist);
}

//-----------------------------------------------------------------------------
// to be implemented
//-----------------------------------------------------------------------------
Int_t TEvdSimParticle::DistancetoPrimitivePhiZ(Int_t px, Int_t py) {

  // static TVector3 global, g2;

  // int dpy (10);

  // TStnVisManager* vm = TStnVisManager::Instance();

  // global.SetXYZ(gPad->AbsPixeltoX(px),gPad->AbsPixeltoY(py),0);
  // g2.SetXYZ    (gPad->AbsPixeltoX(px),gPad->AbsPixeltoY(py+dpy),0);

  // double  dy   = g2.y()-global.y();

  // double z0    = fS1->position().z();
  // double t0    = fmod(fS1->time(),vm->MbTime());

  // double m     = fParticlePDG->Mass()*1e3;     // returned mass is in (GeV)
  // double p     = fS1->momentum().R();
  // double vz    = fS1->momentum().z()/sqrt(p*p+m*m)*300.;  // vz in cm/ns

  // double tz    = fmod(t0+(global.x()-z0)/vz,vm->MbTime());

  // double dt    = global.y()-tz;

  // int dist     = (dt/dy)*dpy; // in pixels
  printf("TEvdSimParticle::DistancetoPrimitivePhiZ not implemented yet. Take your chance to help!\n");
  int dist(1000);
  return abs(dist);
}


//-----------------------------------------------------------------------------
void TEvdSimParticle::Clear(Option_t* Opt) {
  //  SetLineColor(1);
}


//-----------------------------------------------------------------------------
void TEvdSimParticle::Print(Option_t* Option) const {

  //  TAnaDump* tad = TAnaDump::Instance();

  TString opt = Option;
  opt.ToLower();

  // if (opt == "") tad->printSimParticle(fSimp->SimParticle(), "banner+data", nullptr);
  // else           tad->printSimParticle(fSimp->SimParticle(), Option       , nullptr);

  if (opt == "") fSimp->Print("banner+data");
  else           fSimp->Print(Option);

}
}
