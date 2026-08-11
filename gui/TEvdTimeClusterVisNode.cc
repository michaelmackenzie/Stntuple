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

// #include "art/Framework/Principal/Event.h"
// #include "art/Framework/Principal/Handle.h"

#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"
// #include "Offline/TrackerConditions/inc/StrawResponse.hh"

#include "Offline/RecoDataProducts/inc/StrawHit.hh"
#include "Offline/DataProducts/inc/StrawId.hh"

#include "Stntuple/print/TAnaDump.hh"

#include "Stntuple/gui/TEvdComboHit.hh"
#include "Stntuple/gui/TEvdTimeCluster.hh"
#include "Stntuple/gui/TEvdTimeClusterVisNode.hh"
#include "Stntuple/gui/TEvdStraw.hh"
#include "Stntuple/gui/TEvdStrawHit.hh"
#include "Stntuple/gui/TEvdTrkStrawHit.hh"
#include "Stntuple/gui/TEvdStation.hh"
#include "Stntuple/gui/TEvdPanel.hh"
#include "Stntuple/gui/TEvdPlane.hh"
// #include "Stntuple/gui/TEvdTracker.hh"
#include "Stntuple/gui/TEvdSimParticle.hh"
#include "Stntuple/gui/TStnVisManager.hh"

#include "Stntuple/obj/TStnTimeClusterBlock.hh"
#include "Stntuple/obj/TSimpBlock.hh"

ClassImp(TEvdTimeClusterVisNode)

using stntuple::TEvdTimeCluster;

//-----------------------------------------------------------------------------
TEvdTimeClusterVisNode::TEvdTimeClusterVisNode() : TStnVisNode("") {
}

//_____________________________________________________________________________
TEvdTimeClusterVisNode::TEvdTimeClusterVisNode(const char* name, TStnTimeClusterBlock* TcBlock): 
  TStnVisNode(name) {
  fTcBlock            = TcBlock;
	            
  fListOfTimeClusters = new TObjArray();
  fListOfTimeClusters->SetOwner(kTRUE);

  fListOfPhiClusters  = new TObjArray();
  fListOfPhiClusters->SetOwner(kTRUE);

  fTcColl             = nullptr;
  fPcColl             = nullptr;
  fChColl             = nullptr;
  //  fChfColl            = nullptr;
}

//-----------------------------------------------------------------------------
TEvdTimeClusterVisNode::~TEvdTimeClusterVisNode() {
  delete fListOfTimeClusters;
  delete fListOfPhiClusters;
}

//-----------------------------------------------------------------------------
int TEvdTimeClusterVisNode::InitEvent() {

  TStnVisManager* vm = TStnVisManager::Instance();
  const art::Event* event = vm->Event();

//-----------------------------------------------------------------------------
// pointers to collections of interest
//-----------------------------------------------------------------------------
  art::Handle<mu2e::TimeClusterCollection> tccH;
  event->getByLabel(art::InputTag(fTcCollTag), tccH);
  if (tccH.isValid()) fTcColl = tccH.product();
  else {
    mf::LogWarning("TEvdTimeClusterVisNode::InitEvent") << " WARNING:" << __LINE__ 
							<< " : mu2e::TimeClusterCollection " 
							<< fTcCollTag << " not found";
    fTcColl = nullptr;
  }

  art::Handle<mu2e::TimeClusterCollection> pccH;
  event->getByLabel(art::InputTag(fPcCollTag), pccH);
  if (pccH.isValid()) fPcColl = pccH.product();
  else {
    mf::LogWarning("TEvdTimeClusterVisNode::InitEvent") << " WARNING:" << __LINE__ 
							<< " : mu2e::TimeClusterCollection " 
							<< fPcCollTag << " not found";
    fPcColl = nullptr;
  }
//-----------------------------------------------------------------------------
// combo hit and single straw combo hit collections
//-----------------------------------------------------------------------------
  art::Handle<mu2e::ComboHitCollection> chcH;
  event->getByLabel(art::InputTag(fChCollTag), chcH);
  if (chcH.isValid()) fChColl = chcH.product();
  else {
    mf::LogWarning("TEvdTimeClusterVisNode::InitEvent") << " WARNING:" << __LINE__ 
							<< " : mu2e::ComboHitCollection " 
							<< fChCollTag << " not found";
    fChColl = nullptr;
  }

  art::Handle<mu2e::ComboHitCollection> sschcH;
  event->getByLabel(art::InputTag(fSschCollTag), sschcH);
  if (sschcH.isValid()) fSschColl = sschcH.product();
  else {
    mf::LogWarning("TEvdTimeClusterVisNode::InitEvent") << " WARNING:" << __LINE__ 
							<< " : mu2e::ComboHitCollection " 
							<< fSschCollTag << " not found";
    fSschColl = nullptr;
  }
//-----------------------------------------------------------------------------
// initialize time clusters
//-----------------------------------------------------------------------------
  const mu2e::TimeCluster    *tc;  

  fListOfTimeClusters->Delete();

  int ntc = 0;
  if (fTcColl != nullptr) ntc = fTcColl->size();
  
  for (int i=0; i<ntc; i++) {
    tc    = &fTcColl->at(i);
//-----------------------------------------------------------------------------
// time clusters are made out of combo hits, define tmax, tmin
// however, define TMin and TMax by the combined straw hits 
//-----------------------------------------------------------------------------
    int nch    = tc->nhits();
    int nshtot = 0;
    double t0(-1.), tmin(1.e6), tmax(-1), zmin(1.e6), zmax(-1.), phimin(1.e6), phimax(-1.e6);

    if (fChColl != nullptr) {
      double sumt = 0;
      for (int ih=0; ih<nch; ih++) {
	int ind = tc->hits().at(ih);
	const mu2e::ComboHit* hit = &fChColl->at(ind);
        int nsh = hit->nStrawHits();
        nshtot += nsh;
        for (int ish=0; ish<nsh; ish++) {
          // int shind = hit->index(ish);
          // const mu2e::ComboHit* sh = &fSschColl->at(shind);
          float time = hit->correctedTime();
          if (time < tmin) tmin = time;
          if (time > tmax) tmax = time;

          sumt += time;
        }

	float z = hit->pos().z();
	if (z < zmin) zmin = z;
	if (z > zmax) zmax = z;
      }

      t0 = sumt/(nshtot+1e-12);
    }
    else {
      tmin =     0;
      tmax =  1700;
      zmin = -1600;
      zmax =  1600;
    }
      
    stntuple::TEvdTimeCluster* tcl;
    tcl = new stntuple::TEvdTimeCluster(i,tc,fChColl,t0,tmin,tmax,zmin,zmax,phimin,phimax,this);
    fListOfTimeClusters->Add(tcl);
  }
//-----------------------------------------------------------------------------
// phi clusters
//-----------------------------------------------------------------------------
  fListOfPhiClusters->Delete();
  int npc = 0;
  if (fPcColl != nullptr) npc = fPcColl->size();
  
  for (int i=0; i<npc; i++) {
    const mu2e::TimeCluster* pc = &fPcColl->at(i);
//-----------------------------------------------------------------------------
// phi clusters are time clusters ...
//-----------------------------------------------------------------------------   
    double t0(-1.), tmin(1.e6), tmax(-1), zmin(1.e6), zmax(-1.), phimin(1.e6), phimax(-1.e6);

    double sumt = 0;
    if (fChColl != nullptr) {
      int nch = pc->nhits();
      for (int ih=0; ih<nch; ih++) {
	int ind = pc->hits().at(ih);
	const mu2e::ComboHit* hit = &fChColl->at(ind);
	float time = hit->correctedTime();
	if (time < tmin) tmin = time;
	if (time > tmax) tmax = time;

        sumt += time;

        float phi = hit->phi();      // [-M_PI, M_PI]
        if (phi < 0) phi += 2*M_PI;  
                                        // the range to be defined later
        if (phi > phimax) phimax = phi;
        if (phi < phimin) phimin = phi;

	float z = hit->pos().z();
	if (z < zmin) zmin = z;
	if (z > zmax) zmax = z;
      }
      t0 = sumt/(nch+1.e-12);
    }
    else {
      tmin =     0;
      tmax =  1700;
      zmin = -1600;
      zmax =  1600;
    }
      
    stntuple::TEvdTimeCluster* pcl;
    pcl = new stntuple::TEvdTimeCluster(i,pc,fChColl,t0,tmin,tmax,zmin,zmax,phimin,phimax,this);
    fListOfPhiClusters->Add(pcl);
  }

  return 0;
}

//-----------------------------------------------------------------------------
// XY view : display phi clusters , only if a time cluster has been selected
//-----------------------------------------------------------------------------
void TEvdTimeClusterVisNode::PaintXY(Option_t* Option) {

  TStnVisManager* vm = TStnVisManager::Instance();

  TEvdTimeCluster* tc = vm->SelectedTimeCluster();
  if (tc) {
                                        // display phi clusters corresponding to this time cluster

    int ncl = fListOfPhiClusters->GetEntries();
    for (int i=0; i<ncl; i++) {
      TEvdTimeCluster* pc = (TEvdTimeCluster*) fListOfPhiClusters->At(i);
      float t0 = pc->T0();
      if ((t0 <= tc->TMax()) and (t0 >= tc->TMin())) {
        pc->PaintXY(Option);
      }
    }
  }
}

//-----------------------------------------------------------------------------
// TZ view is only for the pattern recognition / time cluster finding
// display reconstructed tracks and combo hits 
// do not display phi clusters in TZ 
//-----------------------------------------------------------------------------
void TEvdTimeClusterVisNode::PaintTZ(Option_t* Option) {

  int ncl = fListOfTimeClusters->GetEntries();
  for (int i=0; i<ncl; i++) {
    TEvdTimeCluster* tcl = (TEvdTimeCluster*) fListOfTimeClusters->At(i);
    tcl->PaintTZ(Option);
  }
}

//-----------------------------------------------------------------------------
void TEvdTimeClusterVisNode::PaintRZ(Option_t *Option) {}

//-----------------------------------------------------------------------------
void TEvdTimeClusterVisNode::PaintPhiZ(Option_t* Option) {}
//-----------------------------------------------------------------------------
void TEvdTimeClusterVisNode::PaintCrv(Option_t* Option) {}
//-----------------------------------------------------------------------------
void TEvdTimeClusterVisNode::PaintCal(Option_t* Option) {}
//-----------------------------------------------------------------------------
void TEvdTimeClusterVisNode::PaintVST(Option_t* Option) {}
//-----------------------------------------------------------------------------
void TEvdTimeClusterVisNode::PaintVRZ(Option_t* Option) {}

//-----------------------------------------------------------------------------
// sets closest object among objects associated with the node 
// returns min_dist to the view
//-----------------------------------------------------------------------------
Int_t TEvdTimeClusterVisNode::DistancetoPrimitiveTZ(Int_t px, Int_t py) {
  TObject* closest(nullptr);
  int      min_dist(9999);

  int ntc = fListOfTimeClusters->GetEntriesFast();
  for (int i=0; i<ntc; i++) {
    TEvdTimeCluster* tcl = EvdTimeCluster(i);
    int dist = tcl->DistancetoPrimitiveTZ(px,py);
    if (dist < min_dist) {
      min_dist = dist;
      closest  = tcl;
    }
  }

  SetClosestObject(closest,min_dist);

  return min_dist;
}


//-----------------------------------------------------------------------------
void TEvdTimeClusterVisNode::Clear(Option_t* Opt) {
  printf(">>> name: %s TEvdTimeClusterVisNode::Clear is not implemented yet\n",GetName());
}

//-----------------------------------------------------------------------------
void TEvdTimeClusterVisNode::NodePrint(const void* Object, const char* ClassName) {
  TString class_name(ClassName);

  TAnaDump* ad = TAnaDump::Instance();

  if (class_name == "TimeCluster") {
//-----------------------------------------------------------------------------
// print a HelixSeed or a HelixSeed collection
//-----------------------------------------------------------------------------
    if (Object) {
      const mu2e::TimeCluster* tc = (const mu2e::TimeCluster*) Object;
      ad->printTimeCluster(tc,"data+hits+banner",fChColl,fSdmcCollTag.encode().data());
    }
    else {
					// Object = nullptr: print collection, with hits 
      ad->printTimeClusterCollection(fTcCollTag.encode().data(),fChCollTag.encode().data(),1,fSdmcCollTag.encode().data());
    }
  }
  else {
    printf("WARNING in TTimeClusterVisNode::%s: print for %s not implemented yet\n",__func__,ClassName);
  }
}

//-----------------------------------------------------------------------------
void TEvdTimeClusterVisNode::Print(Option_t* Opt) const {
  // printf(" >>> name: %s TEvdTimeClusterVisNode::Print is not implemented yet\n",GetName());

  TAnaDump* ad = TAnaDump::Instance();
  ad->printTimeClusterCollection(fTcCollTag.encode().data(),fChCollTag.encode().data(),1,fSdmcCollTag.encode().data());
}

