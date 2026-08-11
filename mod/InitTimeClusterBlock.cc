//-----------------------------------------------------------------------------
//  Apr 2016 G. Pezzullo: initialization of the MU2E STNTUPLE TimePeak block
//  2020-10-18 P.M. rewrite 
//-----------------------------------------------------------------------------
#include <cstdio>
#include "TROOT.h"
#include "TFolder.h"
#include "TLorentzVector.h"
#include "TVector2.h"

#include "messagefacility/MessageLogger/MessageLogger.h"

#include "Stntuple/obj/TStnDataBlock.hh"
#include "Stntuple/obj/TStnEvent.hh"

#include "Stntuple/obj/TStnTimeCluster.hh"
#include "Stntuple/obj/TStnTimeClusterBlock.hh"

#include "Stntuple/obj/TStnHelix.hh"
#include "Stntuple/obj/TStnHelixBlock.hh"

#include "Stntuple/mod/InitTimeClusterBlock.hh"

#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Event.h"

#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"

#include "Offline/CalorimeterGeom/inc/Calorimeter.hh"

#include "Offline/RecoDataProducts/inc/TimeCluster.hh"
#include "Offline/RecoDataProducts/inc/HelixSeed.hh"
#include "Offline/RecoDataProducts/inc/StrawHitIndex.hh"

#include "Offline/MCDataProducts/inc/StrawDigiMC.hh"
#include "Offline/MCDataProducts/inc/StrawGasStep.hh"
#include "Offline/MCDataProducts/inc/SimParticle.hh"

#include "Offline/RecoDataProducts/inc/CaloCluster.hh"
//-----------------------------------------------------------------------------
// assume that the collection name is set, so we could grab it from the event
// ComboHitCollection and StrawHitCollection are of the same time, the first one 
// contains real combo hits (one combo hit could be made out of more than one straw hit),
// the other one has a combo hit per straw digi
//-----------------------------------------------------------------------------
int  StntupleInitTimeClusterBlock::InitDataBlock(TStnDataBlock* Block, AbsEvent* Evt, int Mode) {
  const char* oname = {"StntupleInitTimeClusterBlock::InitDataBlock"};

  TStnTimeClusterBlock*         cb = (TStnTimeClusterBlock*) Block;

  cb->Clear();

  const mu2e::TimeClusterCollection*        list_of_tclusters(nullptr);

  art::Handle<mu2e::TimeClusterCollection>  tccH;
  int                                       ntc(0);

  if (! fTimeClusterCollTag.empty()) {
    bool ok = Evt->getByLabel(fTimeClusterCollTag,tccH);
    if (ok) {
      list_of_tclusters = tccH.product();
      ntc               = list_of_tclusters->size();
    }
  }

  art::Handle<mu2e::ComboHitCollection>    chcH;
  const mu2e::ComboHitCollection*          chc(nullptr);

  // art::Handle<mu2e::ComboHitCollection>    sschcH;
  //  const mu2e::ComboHitCollection*          sschc(nullptr);
//-----------------------------------------------------------------------------
// combohits are only needed for MC-specific purpose
//-----------------------------------------------------------------------------
  if (! fChCollTag.empty()) {
    bool ok = Evt->getByLabel(fChCollTag,chcH);
    if (ok) {
      chc          = chcH.product();
    }
    else {
      mf::LogWarning(oname) << " ERROR: no ComboHitCollection tag=" 
			    << fChCollTag.encode().data() <<  " found. BAIL OUT";
      // return -1;
    }
  }
//-----------------------------------------------------------------------------
// single straw hit collection (also ComboHit's
//-----------------------------------------------------------------------------
  // if (! fShCollTag.empty()) {
  //   bool ok = Evt->getByLabel(fShCollTag,sschcH);
  //   if (ok) {
  //     sschc          = sschcH.product();
  //   }
  // }

  art::Handle<mu2e::StrawDigiMCCollection> sdmccH;
  const mu2e::StrawDigiMCCollection*       mcdigis(nullptr);

  if (! fStrawDigiMCCollTag.empty()) {
    bool ok = Evt->getByLabel(fStrawDigiMCCollTag,sdmccH);
    if (ok) {
      mcdigis = sdmccH.product();
    }
  }

  const mu2e::CaloCluster     *cluster(0);
  const mu2e::TimeCluster     *tmpTCl(0);
 
  if (list_of_tclusters) ntc = list_of_tclusters->size();
  
  for (int i=0; i<ntc; i++) {
    TStnTimeCluster* tc = cb->NewTimeCluster();
    tmpTCl              = &list_of_tclusters->at(i);
    cluster             = tmpTCl->caloCluster().get();
    if (cluster != 0) {
      mu2e::GeomHandle<mu2e::Calorimeter> ch;
      const mu2e::Calorimeter* _calorimeter = ch.get();      
      
      tc->fClusterTime    = cluster->time();
      tc->fClusterEnergy  = cluster->energyDep();
      CLHEP::Hep3Vector         gpos = _calorimeter->diskToMu2e(cluster->diskID(),cluster->cog3Vector());
      CLHEP::Hep3Vector         tpos = _calorimeter->mu2eToTracker(gpos);
      tc->fClusterX       = tpos.x();
      tc->fClusterY       = tpos.y();
      tc->fClusterZ       = tpos.z();
    }

    tc->fTimeCluster  = tmpTCl;
    tc->fNComboHits   = tmpTCl->hits().size();
    tc->fNHits        = tmpTCl->nStrawHits();
    tc->fT0           = tmpTCl->t0()._t0;
    tc->fT0Err        = tmpTCl->t0()._t0err;     
    tc->fPosX         = tmpTCl->position().x();     
    tc->fPosY         = tmpTCl->position().y();     
    tc->fPosZ         = tmpTCl->position().z();
//-----------------------------------------------------------------------------
// loop over combo hits to determine the matching MC particle
//-----------------------------------------------------------------------------
    const mu2e::StrawGasStep* step (nullptr);

    int const max_np(200);
    int     np(0), sim_nh[max_np], sim_id[max_np], pdg_code[max_np]; 

    const mu2e::SimParticle*  simm[max_np];
    simm[0] = nullptr;

    if (mcdigis and chc) {
      for (int ih=0; ih<tc->fNComboHits; ih++) {
        StrawHitIndex hit_index   = tmpTCl->hits().at(ih);
        const mu2e::ComboHit* hit = &chc->at(hit_index);
//-----------------------------------------------------------------------------
// loop over straw hits of one combo hit
//-----------------------------------------------------------------------------
        int nsh = hit->nStrawHits();
        for (int ish=0; ish<nsh; ish++) {
          int ind = hit->index(ish);
          const mu2e::StrawDigiMC* mcdigi = &mcdigis->at(ind);
          
          step = mcdigi->earlyStrawGasStep().get();

          int id(-1);
          const mu2e::SimParticle* sim(nullptr);
          if (step) {
            sim = step->simParticle().get(); 
            id  = sim->id().asInt();
          }
//-----------------------------------------------------------------------------
// accumulate list of sim particle ID's for this time cluster
//-----------------------------------------------------------------------------
          int found = 0;
          for (int ip=0; ip<np; ip++) {
            if (id == sim_id[ip]) {
              found       = 1;
              sim_nh[ip] += 1;
              break;
            }
          }
	
          if (sim && (found == 0)) {
            sim_id  [np] = id;
            simm    [np] = sim;
            pdg_code[np] = sim->pdgId();
            sim_nh  [np] = 1;
            np          += 1;
          }
        }
      }
//-----------------------------------------------------------------------------
// identify time cluster with the particle which produced most hits
//-----------------------------------------------------------------------------
      int ipart  = -1;
      int max_nh = sim_nh[0];
    
      const mu2e::SimParticle* best_sim(nullptr);

      for (int ip=1; ip<np; ip++) {
        if (sim_nh[ip] > max_nh) {
          max_nh = sim_nh[ip];
          ipart  = ip;
        }
      }
    
      if (ipart >= 0) {
        best_sim        = simm    [ipart];
        tc->fSimID      = sim_id  [ipart];
        tc->fPdgID      = pdg_code[ipart];
        tc->fNHitsSimID = max_nh;
        tc->fMcMom      = best_sim->startMomentum().mag() ;
      }
    }
  }

  return 0;
}

//_____________________________________________________________________________
Int_t StntupleInitTimeClusterBlock::ResolveLinks(TStnDataBlock* Block, AbsEvent* AnEvent, int Mode) 
{
  // Mu2e version, do nothing

  Int_t  ev_number, rn_number;

  ev_number = AnEvent->event();
  rn_number = AnEvent->run();

  if (! Block->Initialized(ev_number,rn_number)) return -1;

					// do not do initialize links 2nd time

  if (Block->LinksInitialized()) return 0;

  TStnEvent*                 ev;
  TStnTimeClusterBlock*      hb;

  TStnHelixBlock*            tsb;
  TStnHelix*                 helixseed;

  const mu2e::TimeCluster*   ktcluster, *fktcluster;
  const mu2e::HelixSeed*     kseed;

  char                       short_tcluster_block_name[100];

  ev     = Block->GetEvent();
  hb     = (TStnTimeClusterBlock*) Block;
  
  hb->GetModuleLabel("mu2e::TimeClusterCollection"  , short_tcluster_block_name);

  tsb    = (TStnHelixBlock*) ev->GetDataBlock(short_tcluster_block_name);
  
  int    ntc(0);
  if (hb!=nullptr){
    ntc = hb ->NTimeClusters();
  }
  int    nhelixseed(0);
  if (tsb !=nullptr){
    nhelixseed = tsb->NHelices();
  }

  for (int i=0; i<ntc; ++i){
    TStnTimeCluster* tc = hb->TimeCluster(i);
    ktcluster = tc->fTimeCluster;
    int      helixseedIndex(-1);
    for (int j=0; j<nhelixseed; ++j){
      helixseed   = tsb->Helix(j);
      kseed       = helixseed->fHelix;
      fktcluster  = kseed->timeCluster().get();
      if (fktcluster == ktcluster) {
	helixseedIndex = j;
	break;
      }
    }
    
    if (helixseedIndex < 0) {
      printf(">>> ERROR: TimeClusterFinder timeCluster %i -> no HelixSeed associated\n", i);//FIXME!
	  continue;
    }
    
    tc->SetHelixSeedIndex(helixseedIndex);
  }
//-----------------------------------------------------------------------------
// mark links as initialized
//-----------------------------------------------------------------------------
  hb->fLinksInitialized = 1;

  return 0;
}

