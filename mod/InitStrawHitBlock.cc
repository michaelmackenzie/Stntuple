///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "Stntuple/mod/InitStrawHitBlock.hh"

#include "Offline/RecoDataProducts/inc/ComboHit.hh"
#include "Offline/RecoDataProducts/inc/StrawHit.hh"
#include "Offline/RecoDataProducts/inc/StrawDigi.hh"

#include "Offline/MCDataProducts/inc/StrawDigiMC.hh"
#include "Offline/MCDataProducts/inc/StrawGasStep.hh"


#include <vector>

using std::vector ;
//-----------------------------------------------------------------------------
// in this case AbsEvent is just not used
//-----------------------------------------------------------------------------
namespace stntuple {
int InitStrawHitBlock::InitDataBlock(TStnDataBlock* Block, AbsEvent* Event, int Mode) {

  int ev_number, rn_number, mc_flag(0), nhits(0);

  ev_number = Event->event();
  rn_number = Event->run();
  if (rn_number < 100000) mc_flag = 1;

  if (Block->Initialized(ev_number,rn_number)) return 0;

  TStrawHitBlock* data = (TStrawHitBlock*) Block;
  data->Clear();
//-----------------------------------------------------------------------------
// straw hit information
//-----------------------------------------------------------------------------
  art::Handle<mu2e::StrawHitCollection>             shch;
  const mu2e::StrawHitCollection*                   shc (nullptr);

  art::Handle<mu2e::StrawDigiMCCollection>          sdmcch;
  const mu2e::StrawDigiMCCollection*                sdmcc (nullptr);

  art::Handle<mu2e::StrawDigiCollection>            sdch;
  const mu2e::StrawDigiCollection*                  sdc (nullptr);

//-----------------------------------------------------------------------------
// assume that straw hits and combo hits are created by the same module - why ?
//-----------------------------------------------------------------------------
  if (! fShCollTag.empty() != 0) {
    // bool ok = Event->getByLabel(fShCollTag,chch);
    // if (ok) chc = chch.product();
    
    bool ok = Event->getByLabel(fShCollTag,shch);
    if (ok) { 
      shc   = shch.product();
      nhits = shc->size();
    }
    else {
      mf::LogWarning("InitStrawHitBlock::InitDataBlock") << " ERROR:" << __LINE__ 
							 << " : mu2e::StrawHitCollection " 
							 << fShCollTag.encode().data() 
							 << " not found, BAIL OUT. rc = -1";
      return -1;
    }
  }
  
  if (! fStrawDigiCollTag.empty() != 0) {
    bool ok = Event->getByLabel(fStrawDigiCollTag,sdch);
    if (ok) sdc = sdch.product();

    if (sdc == nullptr) {
      mf::LogWarning("InitStrawHitBlock::InitDataBlock") << " Warning:" << __LINE__ 
							 << " : mu2e::StrawDigiCollection " 
							 << fStrawDigiCollTag.encode().data() 
							 << " not found.";
    }
  }

  if (! fStrawDigiMCCollTag.empty() != 0) {
    bool ok = Event->getByLabel(fStrawDigiMCCollTag,sdmcch);
    if (ok) sdmcc = sdmcch.product();

    if (sdmcc == nullptr) {
      mf::LogWarning("InitStrawHitBlock::InitDataBlock") << " Warning:" << __LINE__ 
							 << " : mu2e::StrawDigiMCCollection " 
							 << fStrawDigiMCCollTag.encode().data() 
							 << " not found.";
    }
  }
//-----------------------------------------------------------------------------
// MC data may not be present ...
//-----------------------------------------------------------------------------
  const mu2e::StrawGasStep* step (nullptr);
  const mu2e::SimParticle* sim;

  TStrawHit*           hit; 

  int   pdg_id, mother_pdg_id, sim_id, gen_id;
  float mc_mom;

  if (rn_number < 100000) mc_flag = 1;

  if (nhits > 0) {

    // const mu2e::ComboHit* ch0 = &chc->at(0);
 
    for (int i=0; i<nhits; i++) {
      const mu2e::StrawHit* sh = &shc->at(i);
      // const mu2e::ComboHit* ch = &chc->at(i);

      int sd_flag = 0;
      if (sdc) sd_flag = *((int*) &sdc->at(i).digiFlag());

      step = nullptr;
      if (sdmcc) {  
	const mu2e::StrawDigiMC* mcdigi = &sdmcc->at(i);
	step = mcdigi->earlyStrawGasStep().get();
      }

      hit = data->NewHit(i);

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
      }
      else {
	pdg_id        = -1;
	mother_pdg_id = -1;
	gen_id        = 0xFF;
	sim_id        = -1;
	mc_mom        = -1.;
      }

      int sid = sh->strawId().asUint16() | (mc_flag << 16);

      // straw hit time is an integer (in ns)

      float time [2], tot[2];

      time[0] = sh->time(mu2e::StrawEnd::cal);
      time[1] = sh->time(mu2e::StrawEnd::hv );

      tot[0] = sh->TOT(mu2e::StrawEnd::cal);
      tot[1] = sh->TOT(mu2e::StrawEnd::hv );
      
      gen_id  = gen_id | (sd_flag << 16) ;

      hit->Set(sid, time, tot, 
	       gen_id, sim_id, 
	       pdg_id, mother_pdg_id, 
	       sh->energyDep(), mc_mom);
    }
//-----------------------------------------------------------------------------
// straw digis and waveforms are produced by the same module
//-----------------------------------------------------------------------------
    if (fWriteSdwf != 0) {
      if (! fStrawDigiCollTag.empty() != 0) {

	art::Handle<mu2e::StrawDigiADCWaveformCollection> sdwfch;
	const mu2e::StrawDigiADCWaveformCollection*       sdwfc(nullptr);

	bool ok = Event->getByLabel(fStrawDigiCollTag,sdwfch);
	if (ok) sdwfc = sdwfch.product();

	if (sdwfc == nullptr) {
	  mf::LogWarning(__func__) << " WARNING in InitStrawHitBlock::" << __func__ << ":" << __LINE__ 
				   << ": StrawDigiADCWaveformCollection:" 
				   << fStrawDigiCollTag.encode().data() << " NOT FOUND, rc = -2";
	  return -2;
	}

	int nwf = sdwfc->size();
	for (int i=0; i<nwf; i++) {
	  const mu2e::StrawDigiADCWaveform* sdwf = &sdwfc->at(i);

	  TStrWaveform* wf = data->NewWaveform(i);
	  wf->Set(sdwf->samples().size(), sdwf->samples().data());
	}
      }
      else {
	mf::LogWarning(__func__) << " WARNING in InitStrawHitBlock::" << __func__ << ":" << __LINE__ 
				 << ": StrawDigiADCWaveformCollection coll tag IS EMPTY, rc = -3" ;
	return -3;
      }
    }
  }

  data->f_RunNumber   = rn_number;
  data->f_EventNumber = ev_number;
  
  return 0;

}


}
