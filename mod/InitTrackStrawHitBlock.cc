///////////////////////////////////////////////////////////////////////////////
// 2014-01-26 P.Murat
///////////////////////////////////////////////////////////////////////////////
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "Stntuple/mod/InitStntupleDataBlocks.hh"
#include "Stntuple/mod/InitTrackStrawHitBlock.hh"
#include "Stntuple/obj/TTrackStrawHitBlock.hh"
#include "Stntuple/obj/AbsEvent.hh"

#include "Offline/RecoDataProducts/inc/StrawHit.hh"
#include "Offline/RecoDataProducts/inc/StrawDigi.hh"

#include "Offline/RecoDataProducts/inc/KalSeed.hh"



#include "Offline/MCDataProducts/inc/SimParticle.hh"
#include "Offline/MCDataProducts/inc/StrawGasStep.hh"
#include "Offline/MCDataProducts/inc/StrawDigiMC.hh"

#include "Offline/GeometryService/inc/GeomHandle.hh"
#include "Offline/TrackerGeom/inc/Tracker.hh"


namespace stntuple {
//-----------------------------------------------------------------------------
int InitTrackStrawHitBlock::InitDataBlock(TStnDataBlock* Block, AbsEvent* _Event, int Mode) {
  const char oname [] = {"stntuple::InitTrackStrawHitBlock::InitDataBlock"};

  int ev_number = _Event->event();
  int rn_number = _Event->run();

  if (Block->Initialized(ev_number,rn_number)) return 0;

  mu2e::GeomHandle<mu2e::Tracker> ttHandle;
  tracker = ttHandle.get();

  TTrackStrawHitBlock* data = (TTrackStrawHitBlock*) Block;
  data->Clear();
//-----------------------------------------------------------------------------
//  straw hit information
//-----------------------------------------------------------------------------
  art::Handle<mu2e::KalSeedCollection>  kscH;
  const mu2e::KalSeedCollection*        ks_coll(nullptr);

  art::Handle<mu2e::ComboHitCollection> shcH;
  const mu2e::ComboHitCollection*       sh_coll(nullptr);

  art::Handle<mu2e::StrawDigiCollection> sdcH;
  const mu2e::StrawDigiCollection*       sd_coll(nullptr);

  art::Handle<mu2e::StrawDigiMCCollection> sdmccH;
  const mu2e::StrawDigiMCCollection*       sdmc_coll(nullptr);

  if (! fShCollTag.empty() != 0) {
    bool ok = _Event->getByLabel(fShCollTag,shcH);
    if (ok) sh_coll = shcH.product();
    else {
      mf::LogWarning(oname) << " ERROR in InitTrackStrawHitBlock: no StrawHitColl with tag=" 
                            << fShCollTag.encode().data() <<  " BAIL OUT";
      return -1;
    }
  }

  if (! fKalSeedCollTag.empty() != 0) {
    bool ok = _Event->getByLabel(fKalSeedCollTag,kscH);
    if (ok) ks_coll = kscH.product();
    else {
      printf(" >>> ERROR in InitTrackStrawHitBlock: no KalSeedColl with tag: %s. BAIL OUT\n",
	     fKalSeedCollTag.encode().data());
      return -1;
    }
  }

  if (! fStrawDigiCollTag.empty() != 0) {
    bool ok = _Event->getByLabel(fStrawDigiCollTag,sdcH);
    if (ok) sd_coll = sdcH.product();
    else {
      printf(" >>> WARNING in InitTrackStrawHitBlock: no StrawDigiColl with tag: %s.\n",
	     fStrawDigiCollTag.encode().data());
    }
  }

  if (! fStrawDigiMCCollTag.empty() != 0) {
    bool ok = _Event->getByLabel(fStrawDigiMCCollTag,sdmccH);
    if (ok) sdmc_coll = sdmccH.product();
    else {
      printf(" >>> ERROR in InitTrackStrawHitBlock: no StrawDigiMCColl with tag: %s. BAIL OUT\n",
	     fStrawDigiMCCollTag.encode().data());
      return -1;
    }
  }
//-----------------------------------------------------------------------------
  int nhits   = sh_coll->size();
  int ntracks = ks_coll->size();
//-----------------------------------------------------------------------------
  const mu2e::StrawGasStep* step (nullptr);
  const mu2e::SimParticle*  sim  (nullptr);

  TTrackStrawHit*      hit; 

  int   pdg_id, mother_pdg_id, sim_id, gen_id;
  float mcdoca, mc_mom;

  data->fNTracks = ntracks;
  data->fNTrackHits->Set(data->fNTracks);
  data->fFirst->Set(data->fNTracks);

  const mu2e::ComboHit* sh(nullptr);

  int nhtot = 0;

  if (nhits > 0) {

    for (int i=0; i<ntracks; i++) {
      const mu2e::KalSeed* ks = &ks_coll->at(i);
      std::vector<mu2e::TrkStrawHitSeed> const* ks_hits = &ks->hits();
      nhits   = ks_hits->size();
    
      (*data->fNTrackHits)[i] = nhits;
      (*data->fFirst)     [i] = nhtot;

      nhtot += nhits;

      for (int ih=0; ih<nhits; ih++) {
	// const mu2e::TrkStrawHitSeed* tsh = static_cast<const mu2e::TrkStrawHitSeed*> (&ks_hits->at(ih));
	const auto* tsh = &ks_hits->at(ih);
	int ind = tsh->index(); // in the list of straw hits
	sh      = &sh_coll->at(ind);

	int sd_flag = 0;
	if (sd_coll) sd_flag = *((int*) &sd_coll->at(ind).digiFlag());

	// mu2e::StrawId const& sid = tsh->strawId();
	// const mu2e::Straw* straw = &tracker->getStraw(sid);  // related to commented out part below
	hit   = data->NewHit();

	const mu2e::StrawDigiMC* sdmc = &sdmc_coll->at(ind);  // loc
	step = sdmc->earlyStrawGasStep().get();

	if (step) {
	  art::Ptr<mu2e::SimParticle> const& simptr = step->simParticle(); 
	  art::Ptr<mu2e::SimParticle> mother = simptr;
	
	  while (mother->hasParent()) mother = mother->parent();

	  sim = mother.get();

	  pdg_id        = simptr->pdgId();
	  mother_pdg_id = sim->pdgId();
	
	  if (simptr->fromGenerator()) gen_id = simptr->genParticle()->generatorId().id();
	  else                         gen_id = 0xFF;
	
	  sim_id        = simptr->id().asInt();
	  mc_mom        = step->momvec().mag();

	  // const CLHEP::Hep3Vector& v1 = straw->getMidPoint();
	  // HepPoint p1(v1.x(),v1.y(),v1.z());
	      
	  // const CLHEP::Hep3Vector& v2 = step->position();
	  // HepPoint    p2(v2.x(),v2.y(),v2.z());
	      
	  // TrkLineTraj trstraw(p1,straw->getDirection()  ,0.,0.);
	  // TrkLineTraj trstep (p2,step->momvec().unit(),0.,0.);
	      
	  // TrkPoca poca(trstep, 0., trstraw, 0.);
	
	  // mcdoca = poca.doca();
	}
	else {
	  pdg_id        = -1;
	  mother_pdg_id = -1;
	  gen_id        = 0xFF;
	  sim_id        = -1;
	  mc_mom        = -1.;
	  mcdoca        = 1.e6;
	}

	gen_id = gen_id | (sd_flag << 16); 
      
	int is_active = tsh->flag().hasAnyProperty(mu2e::StrawHitFlagDetail::active);

	hit->Set(sh->strawId().asUint16(), sh->time(), -1. /*sh->dt()*/, sh->energyDep(),
		 is_active,tsh->ambig(),tsh->driftRadius(),
		 pdg_id, mother_pdg_id, gen_id, sim_id, 
		 mcdoca, mc_mom);
      }
    }
  }

  data->fNStrawHits   = nhtot;
  
  data->f_RunNumber   = rn_number;
  data->f_EventNumber = ev_number;
  
  return 0;
}

}
