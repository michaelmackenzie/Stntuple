///////////////////////////////////////////////////////////////////////////////
// 2022-09-25 P.Murat
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
#include "TBox.h"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"

#include "messagefacility/MessageLogger/MessageLogger.h"

#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"
#include "Offline/TrackerConditions/inc/StrawResponse.hh"

#include "Stntuple/print/TAnaDump.hh"

#include "Stntuple/gui/TEvdTimeCluster.hh"
#include "Stntuple/gui/TEvdComboHit.hh"
#include "Stntuple/gui/TEvdHelixSeed.hh"
#include "Stntuple/gui/TEvdHelixVisNode.hh"
#include "Stntuple/gui/TEvdStraw.hh"
#include "Stntuple/gui/TEvdStrawHit.hh"
#include "Stntuple/gui/TEvdTrkStrawHit.hh"
#include "Stntuple/gui/TEvdStation.hh"
#include "Stntuple/gui/TEvdPanel.hh"
#include "Stntuple/gui/TEvdPlane.hh"
#include "Stntuple/gui/TEvdTracker.hh"
#include "Stntuple/gui/TEvdSimParticle.hh"
#include "Stntuple/gui/TStnVisManager.hh"

#include "Stntuple/obj/TSimpBlock.hh"

#include "Offline/RecoDataProducts/inc/StrawHit.hh"

#include "Offline/DataProducts/inc/StrawId.hh"

ClassImp(TEvdHelixVisNode)

//-----------------------------------------------------------------------------
TEvdHelixVisNode::TEvdHelixVisNode() : TStnVisNode("") {
}

//_____________________________________________________________________________
TEvdHelixVisNode::TEvdHelixVisNode(const char* name, TStnHelixBlock* HelixBlock): 
  TStnVisNode(name) {
  fHelixBlock       = HelixBlock;
	            
  fArc              = new TArc;
  //  fEventTime        = 0;
  fTimeWindow       = 1.e6;

  fListOfHelixSeeds = new TObjArray();
  fTimeCluster      = NULL;


  fHsColl           = nullptr;
}

//-----------------------------------------------------------------------------
TEvdHelixVisNode::~TEvdHelixVisNode() {
  delete fArc;
  
  fListOfHelixSeeds->Delete();
  delete fListOfHelixSeeds;
}

//-----------------------------------------------------------------------------
int TEvdHelixVisNode::InitEvent() {

  TStnVisManager* vm = TStnVisManager::Instance();

  const art::Event* event = vm->Event();

  art::Handle<mu2e::HelixSeedCollection> hscH;
  event->getByLabel(art::InputTag(fHsCollTag), hscH);
  if (hscH.isValid()) fHsColl = hscH.product();
  else {
    mf::LogWarning("TEvdHelixVisNode::InitEvent") << " WARNING:" << __LINE__ 
						  << " : mu2e::HelixSeedCollection " 
						  << fHsCollTag << " not found";
    fHsColl = nullptr;
  }


  //  const mu2e::ComboHit              *hit;
  // stntuple::TEvdStrawHit            *evd_straw_hit; 
  // const CLHEP::Hep3Vector           *w; 
  // const mu2e::Straw                 *straw; 

  // int                               nl, ns; // , ipeak, ihit;
  // bool                              isFromConversion, intime;
  // double                            sigw(1000.), /*vnorm, v,*/ sigr; 
  CLHEP::Hep3Vector                 vx0, vx1, vx2;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// initialize helices
//-----------------------------------------------------------------------------
  stntuple::TEvdHelixSeed  *hel;
  const mu2e::HelixSeed    *hseed;  

  fListOfHelixSeeds->Delete();
  int nhel = 0;

  if (fHsColl != nullptr) nhel = fHsColl->size();
  
  for (int i=0; i<nhel; i++) {
    hseed = &fHsColl->at(i);
    hel   = new stntuple::TEvdHelixSeed(i,hseed, this);
//-----------------------------------------------------------------------------
// helices are made out of combo hits
//-----------------------------------------------------------------------------
    // const mu2e::ComboHitCollection* hits = &hseed->hits();
    // for (auto it=hits->begin(); it!=hits->end(); it++) {
    //   const mu2e::ComboHit* hit = (*it);
    //   if (hit == nullptr) continue;
    //   stntuple::TEvdTrkStrawHit* h = new stntuple::TEvdTrkStrawHit(track_hit);
    //   trk->AddHit(h);
    // }

    fListOfHelixSeeds->Add(hel);
  }

  return 0;
}

//-----------------------------------------------------------------------------
// draw reconstructed tracks and STRAW hits, may want to display COMBO hits instead
//-----------------------------------------------------------------------------
void TEvdHelixVisNode::PaintXY(Option_t* Option) {

  // double                  time;
  // int                     station, ntrk(0);

  // const mu2e::Straw      *straw; 

  
  //  int view_type = TVisManager::Instance()->GetCurrentView()->Type();

  // mu2e::GeomHandle<mu2e::Tracker> ttHandle;
  // const mu2e::Tracker* tracker = ttHandle.get();

  TStnVisManager* vm = TStnVisManager::Instance();

  stntuple::TEvdTimeCluster* tcl = vm->SelectedTimeCluster();

  double tmin(0), tmax(2000.);

  if (tcl) {
    tmin = tcl->TMin();
    tmax = tcl->TMax();
  }
  
//-----------------------------------------------------------------------------
// now - helices
//-----------------------------------------------------------------------------
  if (vm->DisplayHelices()) {
    int nhel(0);
    if (fListOfHelixSeeds)  nhel = fListOfHelixSeeds->GetEntriesFast();

    for (int i=0; i<nhel; i++ ) {
      stntuple::TEvdHelixSeed* hel = GetEvdHelixSeed(i);
      double t0 = hel->T0();
      if ((t0 > tmin) and (t0 < tmax)) hel->Paint(Option);
    }
  }
  gPad->Modified();
}

//-----------------------------------------------------------------------------
// in RZ view can display only straw hits, they are painted by fTracker
//-----------------------------------------------------------------------------
void TEvdHelixVisNode::PaintRZ(Option_t* Option) {
  // int             ntrk(0), nhits;
  //  stntuple::TEvdTrack*      evd_trk;

  //  TStnVisManager* vm = TStnVisManager::Instance();

  //  fTracker->PaintRZ(Option);
//-----------------------------------------------------------------------------
// do not draw all straw hits - just redraw straws in different color instead
//-----------------------------------------------------------------------------
//   int nhits = fListOfStrawHits->GetEntries();
//   for (int i=0; i<nhits; i++) {
//     hit       = (TEvdStrawHit*) fListOfStrawHits->At(i);

//     if ((station >= vm->MinStation()) && (station <= vm->MaxStation())) continue;
//     if ((time    <  vm->TMin()      ) || (time     > vm->TMax()      )) continue; 

//     hit->Paint(Option);
//   }
//-----------------------------------------------------------------------------
// display tracks and track hits
//-----------------------------------------------------------------------------
//   if (fListOfTracks != 0)  ntrk = fListOfTracks->GetEntriesFast();

//   for (int i=0; i<ntrk; i++ ) {
//     evd_trk = (stntuple::TEvdTrack*) fListOfTracks->At(i);
//     evd_trk->Paint(Option);

//     nhits = evd_trk->NHits();
//     for (int ih=0; ih<nhits; ih++) {
//       stntuple::TEvdTrkStrawHit* hit = evd_trk->Hit(ih);
//       hit->PaintRZ(Option);
//     }
//   }

// //-----------------------------------------------------------------------------
// // SimParticle's : pT at  the ST is too large, need to use parameters at the tracker entrance ?
// //-----------------------------------------------------------------------------
//   stntuple::TEvdSimParticle* esim;
//   int nsim(0);

//   if ( (fListOfSimParticles) != 0 )  nsim = fListOfSimParticles->GetEntriesFast();

//   for (int i=0; i<nsim; i++ ) {
//     esim = (stntuple::TEvdSimParticle*) fListOfSimParticles->At(i);
//     esim->PaintRZ(Option);
//   }

//   gPad->Modified();
}


//-----------------------------------------------------------------------------
// TZ view is only for the pattern recognition / time cluster finding
// display reconstructed tracks and combo hits 
//-----------------------------------------------------------------------------
void TEvdHelixVisNode::PaintTZ(Option_t* Option) {

  // int nhits = fListOfComboHits->GetEntries();
  // for (int i=0; i<nhits; i++) {
  //   stntuple::TEvdComboHit* hit = (stntuple::TEvdComboHit*) fListOfComboHits->At(i);

  //   // float time  = hit->Time();

  //   //    if ((time >= tmin) && (time <= tmax)) {
  //   hit->PaintTZ(Option);
  //   // }
  // }

//-----------------------------------------------------------------------------
// SimParticle's
//-----------------------------------------------------------------------------
  // stntuple::TEvdSimParticle* esim;
  // int nsim(0);

  // if ( (fListOfSimParticles) != 0 )  nsim = fListOfSimParticles->GetEntriesFast();

  // for (int i=0; i<nsim; i++ ) {
  //   esim = (stntuple::TEvdSimParticle*) fListOfSimParticles->At(i);
  //   esim->PaintTZ(Option);
  // }

  // gPad->Modified();
}

//-----------------------------------------------------------------------------
// VST view : display all straws 
//-----------------------------------------------------------------------------
void TEvdHelixVisNode::PaintVST(Option_t* Option) {

  // gPad->Modified();
}

//-----------------------------------------------------------------------------
void TEvdHelixVisNode::PaintPhiZ(Option_t* Option) {
}
//-----------------------------------------------------------------------------
void TEvdHelixVisNode::PaintCal(Option_t *Option) {
}
//-----------------------------------------------------------------------------
void TEvdHelixVisNode::PaintCrv(Option_t *Option) {
}

//-----------------------------------------------------------------------------
void TEvdHelixVisNode::PaintVRZ(Option_t *Option) {
}

//------------------------------------
int TEvdHelixVisNode::DistancetoPrimitiveXY(Int_t px, Int_t py) {
  static TVector3 global;
  global.SetXYZ(gPad->AbsPixeltoX(px),gPad->AbsPixeltoY(py),0);

  TObject* closest(nullptr);

  int  min_dist(9999), dist;
  TStnVisManager* vm = TStnVisManager::Instance();
//-----------------------------------------------------------------------------
// helices are represented by ellipses
//-----------------------------------------------------------------------------
  if (vm->DisplayHelices()) {
    int nhel = fListOfHelixSeeds->GetEntriesFast();
    for (int i=0; i<nhel; i++) {
      stntuple::TEvdHelixSeed* hel = GetEvdHelixSeed(i);

      dist = hel->DistancetoPrimitiveXY(px,py);

      if (dist < min_dist) {
	min_dist = dist;
	closest  = hel;
      }
    }

    SetClosestObject(closest,min_dist);
  }

  return min_dist;
}

//-----------------------------------------------------------------------------
Int_t TEvdHelixVisNode::DistancetoPrimitiveRZ(Int_t px, Int_t py) {
  return 9999;
}


//-----------------------------------------------------------------------------
Int_t TEvdHelixVisNode::DistancetoPrimitiveTZ(Int_t px, Int_t py) {

  int min_dist(9999);

//   // static TVector3 global;
//   // global.SetXYZ(gPad->AbsPixeltoX(px),gPad->AbsPixeltoY(py),0);

//   TObject* closest(nullptr);

//   int  x1, y1, dx1, dy1, dist;

//   int nhits = fListOfComboHits->GetEntries();
//   for (int i=0; i<nhits; i++) {
//     stntuple::TEvdComboHit* hit = (stntuple::TEvdComboHit*) fListOfComboHits->At(i);
//     x1  = gPad->XtoAbsPixel(hit->Z());
//     y1  = gPad->YtoAbsPixel(hit->T());
//     dx1 = px-x1;
//     dy1 = py-y1;

//     dist  = (int) sqrt(dx1*dx1+dy1*dy1);
//     if (dist < min_dist) {
//       min_dist = dist;
//       closest  = hit;
//     }
//   }
// //-----------------------------------------------------------------------------
// // simparticles are represented by lines
// //-----------------------------------------------------------------------------
//   int nsim = fListOfSimParticles->GetEntries();
//   for (int i=0; i<nsim; i++) {
//     stntuple::TEvdSimParticle* esp = GetEvdSimParticle(i);

//     dist = esp->DistancetoPrimitiveTZ(px,py);

//     if (dist < min_dist) {
//       min_dist = dist;
//       closest  = esp;
//     }
//   }

//   SetClosestObject(closest,min_dist);

  return min_dist;
}


//-----------------------------------------------------------------------------
void TEvdHelixVisNode::Clear(Option_t* Opt) {
  printf(">>> name: %s TEvdHelixVisNode::Clear is not implemented yet\n",GetName());
}

//-----------------------------------------------------------------------------
void TEvdHelixVisNode::NodePrint(const void* Object, const char* ClassName) {
  TString class_name(ClassName);

  TAnaDump* ad = TAnaDump::Instance();

  if (class_name == "HelixSeed") {
//-----------------------------------------------------------------------------
// print a HelixSeed or a HelixSeed collection
//-----------------------------------------------------------------------------
    if (Object) {
      const mu2e::HelixSeed* hs = (const mu2e::HelixSeed*) Object;
      ad->printHelixSeed(hs,"",fShCollTag.encode().data(),fSdmcCollTag.encode().data());
    }
    else {
					// Object = nullptr: print collection, with hits 
      ad->printHelixSeedCollection(fHsCollTag.encode().data(),1,fShCollTag.encode().data(),fSdmcCollTag.encode().data());
    }
  }
  else {
    printf("WARNING in TEvdHelixVisNode::NodePrint: print for %s not implemented yet\n",ClassName);
  }
}

//-----------------------------------------------------------------------------
void TEvdHelixVisNode::Print(Option_t* Opt) const {
  printf(" >>> name: %s TEvdHelixVisNode::Print is not implemented yet\n",GetName());

//-----------------------------------------------------------------------------
// print helix seeds
//-----------------------------------------------------------------------------
  int banner_printed(0);
  int nhel = fListOfHelixSeeds->GetEntries();
  printf("n(helix seeds) = %i\n",nhel);

  for (int i=0; i<nhel; i++) {
    stntuple::TEvdHelixSeed* hel = (stntuple::TEvdHelixSeed*) fListOfHelixSeeds->At(i);
    if (banner_printed == 0) {
      hel->Print("banner");
      banner_printed = 1;
    }
    hel->Print("data");
  }

}

