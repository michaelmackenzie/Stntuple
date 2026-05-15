///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
#include "TLorentzVector.h"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"
#include <set>
#include <vector>

#include "messagefacility/MessageLogger/MessageLogger.h"

#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Selector.h"
#include "canvas/Utilities/InputTag.h"

#include "Offline/GlobalConstantsService/inc/GlobalConstantsHandle.hh"
#include "Offline/GlobalConstantsService/inc/ParticleDataList.hh"

#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"
#include "Offline/GeometryService/inc/VirtualDetector.hh"

#include "Offline/DataProducts/inc/VirtualDetectorId.hh"

#include "Offline/MCDataProducts/inc/CaloClusterMC.hh"
#include "Offline/MCDataProducts/inc/CaloEDepMC.hh"
#include "Offline/MCDataProducts/inc/GenParticle.hh"
#include "Offline/MCDataProducts/inc/KalSeedMC.hh"
#include "Offline/MCDataProducts/inc/SimParticle.hh"
#include "Offline/MCDataProducts/inc/StepPointMC.hh"
#include "Offline/MCDataProducts/inc/StrawDigiMC.hh"
#include "Offline/MCDataProducts/inc/PrimaryParticle.hh"

#include "Offline/RecoDataProducts/inc/StrawHit.hh"

#include "Stntuple/obj/TSimpBlock.hh"

#include "Stntuple/mod/StntupleUtilities.hh"
#include "Stntuple/mod/InitStntupleDataBlocks.hh"


// #include "Stntuple/mod/THistModule.hh"
#include "Stntuple/base/TNamedHandle.hh"

#include "Stntuple/mod/InitSimpBlock.hh"

#include "messagefacility/MessageLogger/MessageLogger.h"

//-----------------------------------------------------------------------------
// fill SimParticle's data block
//-----------------------------------------------------------------------------
int StntupleInitSimpBlock::InitDataBlock(TStnDataBlock* Block, AbsEvent* AnEvent, int Mode) {
  const char* oname = {"Stntuple::InitSimpBlock"};

  const mu2e::SimParticleCollection*       simp_coll(nullptr);
  const mu2e::SimParticle*                 sim      (nullptr);
  const mu2e::StrawHitCollection*          shColl   (nullptr);
  const mu2e::StrawDigiMCCollection*       mcdigis  (nullptr);

  double        px, py, pz, energy;
  int           id, parent_id, process_id, nsh(0), nhits;
  int           pdg_code, start_vol_id, end_vol_id, creation_code, termination_code;
  TSimParticle* simp;

  TSimpBlock* simp_block = (TSimpBlock*) Block;
  simp_block->Clear();

  auto const& ptable = mu2e::GlobalConstantsHandle<mu2e::ParticleDataList>();

  art::Handle<mu2e::StrawHitCollection> shcH;
  if (! fShCollTag.empty()) {
    bool ok = AnEvent->getByLabel(fShCollTag,shcH);
    if (ok) {
      shColl = shcH.product();
      nsh    = shColl->size();
    }
    else {
      mf::LogWarning(oname) << " WARNING line " << __LINE__ << ": no StrawHitCollection tag=" 
			    << fShCollTag.encode().data() <<  " found";
    }
  }

  art::Handle<mu2e::StrawDigiMCCollection> mcdH;

  if (! fStrawDigiMCCollTag.empty()) {
    bool ok = AnEvent->getByLabel(fStrawDigiMCCollTag,mcdH);
    if (ok) mcdigis = mcdH.product();
    else {
      mf::LogWarning(oname) << " WARNING line " << __LINE__ << ": no StrawDigiMCCollection tag=" 
			    << fShCollTag.encode().data() <<  " found";
    }
  }

  art::Handle<mu2e::SimParticleCollection> simp_handle;
  if (! fSimpCollTag.empty()) {
    bool ok = AnEvent->getByLabel(fSimpCollTag,simp_handle);
    if (! ok) {
      mf::LogWarning(oname) << " WARNING line " << __LINE__ 
			    << ": SimpCollection:" 
			    << fSimpCollTag.encode().data() << " NOT FOUND";
      return -1;
    }
  }

  mu2e::GeomHandle<mu2e::VirtualDetector> vdg;

  int nvdhits(0);
  art::Handle<mu2e::StepPointMCCollection> vdhcH;
  const mu2e::StepPointMCCollection*       vdhc(nullptr);

  if (! fVDHitsCollTag.empty()) {
    AnEvent->getByLabel(fVDHitsCollTag,vdhcH);
    if (!vdhcH.isValid()) {
      mf::LogWarning(oname) << " WARNING in " << oname << ":" << __LINE__ 
                            << ": StepPointMCCollection:" 
                            << fVDHitsCollTag.encode().data() << " NOT FOUND";
    }
    else {
      vdhc    = vdhcH.product();
      nvdhits = vdhc->size();
    }
  }
//-----------------------------------------------------------------------------
// figure out how many straw hits each particle has produced
// StrawHitCollection, StrawDigiCollection, and StrawDigiMCCollection are all parallel,
// indexed with the same index
//-----------------------------------------------------------------------------
  std::vector<int>  vid, vin;
  int               np_with_straw_hits(0);  // number of particles with straw hits
                                            // straw hit ID's, per particle
  std::vector< std::vector<int>* > vshid(nsh+1);

  vid.reserve(nsh);
  vin.reserve(nsh);

  for (int i=0; i<nsh; i++) {
    vin  [i] = 0;
    vshid[i] = nullptr;
  }

  if ((nsh > 0) and (mcdigis != nullptr)) {

    const mu2e::StrawGasStep* step(nullptr);

    for (int i=0; i<nsh; i++) {
      const mu2e::StrawDigiMC* mcdigi = &mcdigis->at(i);

      step = mcdigi->earlyStrawGasStep().get();
      if (step) {
//------------------------------------------------------------------------------
// looking at the ppbar annihilation events - 
// searching for the mother works for delta-electrons, but not otherwise, do we really need 
// that? - it worked before, though... comment out for the time being
//-----------------------------------------------------------------------------
	art::Ptr<mu2e::SimParticle> const& simptr = step->simParticle(); 
	art::Ptr<mu2e::SimParticle> mother        = simptr;
	//	while(mother->hasParent())  mother        = mother->parent();
	const mu2e::SimParticle*    sim           = mother.get();
	  
	int sim_id = sim->id().asInt();

	int found  = 0;
	for (int ip=0; ip<np_with_straw_hits; ip++) {
	  if (sim_id == vid[ip]) {
	    vin[ip] += 1;
	    vshid[ip]->push_back(i);
	    found    = 1;
	    break;
	  }
	}

	if (found == 0) {
	  vshid[np_with_straw_hits]      = new std::vector<int>;
	  vshid[np_with_straw_hits]->push_back(i);
	  vid  [np_with_straw_hits]      = sim_id;
	  vin  [np_with_straw_hits]      = 1;
	  np_with_straw_hits             = np_with_straw_hits+1;
	}
      }
    }
  }
//-----------------------------------------------------------------------------
// figure out the primary particle
//-----------------------------------------------------------------------------
  art::Handle<mu2e::PrimaryParticle> pp_handle;
  const mu2e::PrimaryParticle*       pp(nullptr);
  const mu2e::SimParticle*           primary(nullptr);
  const int verbose(0);

  if (! fPrimaryParticleTag.empty()) {
    AnEvent->getByLabel(fPrimaryParticleTag,pp_handle);

    if (pp_handle.isValid()) {
      pp            = pp_handle.product();
      if (!pp->primarySimParticles().empty()) {
        auto front = pp->primarySimParticles().front();
        if(front) {
          primary       = front.get();
          fGenProcessID = primary->creationCode();
          fPdgID        = primary->pdgId();
          if(verbose > 0) {
            printf("Primary particles:\n");
            for(auto sim : pp->primarySimParticles()) printf(" ProcessCode = %20s, PDG = %5i\n", sim->creationCode().name().c_str(), sim->pdgId());
          }
        } else {
          printf("InitSimpBlock::%s: Primary particle (collection %s) sim particle is not valid\n", __func__, fPrimaryParticleTag.encode().c_str());
          pp = nullptr;
        }
      }
    }
  }

  // Retrieve all tracks in the event to check against the sim particles
  auto track_mc_colls = AnEvent->getMany<mu2e::KalSeedMCCollection>();

  // check if this is an RPC event
  const bool is_rpc = primary && (fGenProcessID == mu2e::ProcessCode::mu2eExternalRPC || fGenProcessID == mu2e::ProcessCode::mu2eInternalRPC);

//-----------------------------------------------------------------------------
// build a set of sim particle IDs that deposit at least 50 MeV in any calo cluster
//-----------------------------------------------------------------------------
  constexpr float kMinCaloEDep = 50.f; // MeV

  std::set<int> simp_ids_with_calo_edep;

  if (! fCaloClusterMCCollTag.empty()) {
    art::Handle<mu2e::CaloClusterMCCollection> calo_mc_handle;
    bool ok = AnEvent->getByLabel(fCaloClusterMCCollTag, calo_mc_handle);
    if (ok && calo_mc_handle.isValid()) {
      const mu2e::CaloClusterMCCollection* calo_mc_coll = calo_mc_handle.product();
      for (const auto& mc_cl : *calo_mc_coll) {
        // accumulate energy per sim particle across all hits in this cluster
        std::map<int, float> sim_edep;
        for (const auto& hit : mc_cl.caloHitMCs()) {
          for (const auto& edep : hit->energyDeposits()) {
            const auto sim_ptr = edep.sim();
            if (sim_ptr) {
              sim_edep[sim_ptr->id().asInt()] += edep.energyDep();
            }
          }
        }
        for (const auto& entry : sim_edep) {
          if (entry.second >= kMinCaloEDep) {
            if(verbose > 0) printf("InitSimpBlock::%s: Calo SIM ID = %4i deposits %6.1f MeV in cluster\n",
                                   __func__, entry.first, entry.second);
            simp_ids_with_calo_edep.insert(entry.first);
          }
        }
      }
    }
    else {
      mf::LogWarning(oname) << " WARNING line " << __LINE__ << ": no CaloClusterMCCollection tag="
                            << fCaloClusterMCCollTag.encode().data() << " found";
    }
  }

  if (simp_handle.isValid()) {
    simp_coll = simp_handle.product();

    for (mu2e::SimParticleCollection::const_iterator ip = simp_coll->begin(); ip != simp_coll->end(); ip++) {
      sim      = &ip->second;

      id        = sim->id().asInt();
      parent_id = -1;
//------------------------------------------------------------------------------
// count number of straw hits produced by the particle
//-----------------------------------------------------------------------------
      nhits  = 0;
      std::vector<int>* v_shid(nullptr);

      for (int i=0; i<np_with_straw_hits; i++) {
	if (vid[i] == id) {
	  nhits    = vin  [i];
	  v_shid   = vshid[i];
	  vshid[i] = nullptr;
	  break;
	}
      }
//-----------------------------------------------------------------------------
// need to store e+ and e- from an external photon conversion
// it is possible that will need to lop over the primary particles
//-----------------------------------------------------------------------------
      pdg_code           = (int) sim->pdgId();
      process_id         = sim->creationCode();

      const bool is_pion = std::abs(pdg_code) == 211;
      const bool is_pbar = pdg_code == -2212;

      bool accepted = false; // whether or not to accept the sim

      // Keep all primaries
      bool is_primary = false;
      if (pp != nullptr) {
        bool found = false;
        for (auto pr : pp->primarySimParticles()) {
          if (pr.get() == sim) {
            found = true;
            break;
          }
        }
        is_primary = found;
      }
      accepted |= is_primary;

      // check a for important Process codes that may not get labeled "PrimaryParticle"
      accepted |= process_id == mu2e::ProcessCode::mu2eGammaConversion;
      accepted |= process_id == mu2e::ProcessCode::mu2eInternalRMC    ;
      accepted |= process_id == mu2e::ProcessCode::mu2eExternalRMC    ;
      accepted |= process_id == mu2e::ProcessCode::mu2eInternalRPC    ;
      accepted |= process_id == mu2e::ProcessCode::mu2eExternalRPC    ;
      accepted |= process_id == mu2e::ProcessCode::mu2eFlatPhoton     ;
      accepted |= is_pion && is_rpc                                   ; // save pions for reweighting RPC
      accepted |= is_pbar                                             ; // save pbar for reweighting

      // Check if this is a relevant particle for tracking studies
      constexpr float min_relevant_mom = 30.; // the point at which we may reconstruct the track
      constexpr int   min_nhits        = 12;  // minimum number of hits to be relevant to reconstruction
      const bool relevant_particle = sim->startMomentum().vect().mag() > min_relevant_mom
        && (std::abs(pdg_code) == 11 || std::abs(pdg_code) == 13 || std::abs(pdg_code) == 211);
      const bool relevant_track = (nhits > min_nhits && relevant_particle);
      if(verbose > 0 && !accepted && relevant_track)
        printf("InitSimpBlock::%s: Relevant Trk SIM: ID = %4i, PDG = %5i, Code = %s\n",
               __func__, id, pdg_code, sim->creationCode().name().c_str());
      accepted |= relevant_track;

      // If not yet found but reasonable momentum, check if there is a reconstructed track associated with this SimParticle
      if (!accepted && relevant_particle) {
        for (const auto& coll : track_mc_colls) {
          for (const auto& track : *coll) {
            int nhits = 0;
            for(auto& hit : track.trkStrawHitMCs()) {
              auto stub = track.simParticle(hit);
              if(stub._spkey == sim->id()) nhits++;
              accepted |= nhits > min_nhits;
              if(accepted) break;
            }
            if(accepted) break;
          }
          if(accepted) break;
        }
      }

      // Accept sim particles that deposit at least 50 MeV in a calorimeter cluster
      if (!accepted && simp_ids_with_calo_edep.count(id)) {
        if(verbose > 0) printf("InitSimpBlock::%s: Accepting SIM ID = %4i, PDG = %5i via calo cluster energy deposit\n",
                               __func__, id, pdg_code);
        accepted = true;
      }

      if(verbose > 1) printf("InitSimpBlock::%s: Checking SIM: ID = %4i, PDG = %5i, Code = %s --> accepted = %o\n",
                             __func__, id, pdg_code, sim->creationCode().name().c_str(), accepted);

      if (!accepted)                                     continue;

      if(verbose) printf("InitSimpBlock::%s: Accepting SIM: ID = %4i, PDG = %5i, Code = %s, N(hits) = %2i, p(start) = %6.1f, t(start) = %7.1f, t(end) = %7.1f\n",
                         __func__, id, pdg_code, sim->creationCode().name().c_str(), nhits,
                         sim->startMomentum().vect().mag(), std::fmod(sim->startGlobalTime(), 1695), std::fmod(sim->endGlobalTime(), 1695));

//-----------------------------------------------------------------------------
// if primary particle is not defined, or defined incorrectly (!) 
// store all particles
//-----------------------------------------------------------------------------
      creation_code    = sim->creationCode();
      termination_code = sim->stoppingCode();

      start_vol_id     = sim->startVolumeIndex();
      end_vol_id       = sim->endVolumeIndex();
      
      px               = sim->startMomentum().x();
      py               = sim->startMomentum().y();
      pz               = sim->startMomentum().z();
      // double ptot      = sim->startMomentum().vect().mag();
      energy           = sim->startMomentum().e();
//-----------------------------------------------------------------------------
// more on parent ID
//-----------------------------------------------------------------------------
      const mu2e::SimParticle* parent = sim->parent().get();
      if (parent != nullptr) {
//-----------------------------------------------------------------------------
// try to find parent among already stored in SimpBlock particles
//-----------------------------------------------------------------------------
        int np = simp_block->NParticles();
        for (int i=np-1; i>=0; i--) {
          TSimParticle* p = simp_block->Particle(i);
          if (p->SimParticle() == parent) {
            parent_id = i;
            break;
          }
        }
      }
//-----------------------------------------------------------------------------
// by default, do not store low energy SimParticles not making hits in the tracker
//-----------------------------------------------------------------------------
      const CLHEP::Hep3Vector sp = sim->startPosition();

      // if(!is_pion && !is_pbar) { //no additional requirements for pions needed for reweighting
      //   if ((fMinSimpMomentum >= 0) and (ptot < fMinSimpMomentum)) continue;
      //   if ((fMinNStrawHits   >= 0) and (nhits < fMinNStrawHits )) continue;
      // }

      const int creation_and_primary = (creation_code & 0xfff) | (is_primary << 16); // pack creation code and is primary flag together
      simp   = simp_block->NewParticle(id, parent_id, pdg_code        , 
				       creation_and_primary, termination_code,
				       start_vol_id , end_vol_id      ,
				       process_id);
      simp->SetStartMom(px, py, pz, energy);
      simp->SetStartPos(sp.x(),sp.y(),sp.z(),sim->startGlobalTime());
//-----------------------------------------------------------------------------
// proper time: if StepPointMC collection is defined, make an attempt to correct 
// things for multistage simulation, where the proper time is calculated 
// for each stage separately
//-----------------------------------------------------------------------------
      // double tau               = gc->getParticleLifetime(sim->pdgId());
      double tau = ptable->particle(sim->pdgId()).lifetime()*1.e9;  // convert to ns
      if (tau == 0) tau = 1.e20;

      double start_proper_time = sim->startProperTime();
      double dpt               = stntuple::get_proper_time(sim);

      start_proper_time = start_proper_time+dpt;

      simp->SetStartProperTime(start_proper_time/tau);

      simp->SetEndMom  (sim->endMomentum().x(),
			sim->endMomentum().y(),
			sim->endMomentum().z(),
			sim->endMomentum().e());
      const CLHEP::Hep3Vector ep = sim->endPosition();
      simp->SetEndPos(ep.x(),ep.y(),ep.z(),sim->endGlobalTime());

      double end_proper_time = sim->endProperTime();

      end_proper_time = end_proper_time+dpt;

      simp->SetEndProperTime(end_proper_time/tau);

      simp->SetNStrawHits(nhits);
      simp->SetSimStage(sim->simStage());
      simp->SetSimParticle(sim);
      simp->SetShid(v_shid);
//-----------------------------------------------------------------------------
// particle parameters at virtual detectors -stored only for those which have 
// VD hits stored
//-----------------------------------------------------------------------------
      for (int i=0; i<nvdhits; i++) {
        const mu2e::StepPointMC* hit = &(*vdhc)[i];
	    
        mu2e::VirtualDetectorId vdid(hit->volumeId());
	    
        if (vdid.id() == mu2e::VirtualDetectorId::ST_Out) {
	      
          const mu2e::SimParticle* sim = hit->simParticle().get();
	      
          if (sim == NULL) {
            printf(">>> ERROR: %s sim == NULL\n",oname);
          }
          int sim_id = sim->id().asInt();
          if (sim_id == id) {
            simp->SetMomTargetEnd(hit->momentum().mag());
          }
        }
        else if (vdid.isTrackerFront()) {
          art::Ptr<mu2e::SimParticle> const& simptr = hit->simParticle();
          const mu2e::SimParticle* sim = simptr.get();
	  
          if (sim == NULL) {
            printf("[%s] ERROR: sim == NULL. CONTINUE.\n",oname);
          }
          else {
            int sim_id = sim->id().asInt();
            if (sim_id == id) {
              simp->SetMomTrackerFront(hit->momentum().mag());
            }
          }
        }
	
      }
    }
//-----------------------------------------------------------------------------
// memory clean up , reassigned ones will be deleted together with particles ? 
//-----------------------------------------------------------------------------
    for (int i=0; i<np_with_straw_hits; i++) {
      if (vshid[i]) delete vshid[i];
    }
  }
  else {
//-----------------------------------------------------------------------------
// no SIMP collection
//-----------------------------------------------------------------------------
    mf::LogWarning(oname) << " WARNING in " << oname << ":" << __LINE__ 
			  << " : SimParticleCollection " 
			  << fSimpCollTag.encode() << " not found";
    return -1;
  }

  return 0;
}
