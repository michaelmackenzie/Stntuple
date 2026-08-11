////////////////////////////////////////////////////////////////////////////////
#include "Stntuple/print/TAnaDump.hh"
#include "Stntuple/print/Stntuple_print_functions.hh"

#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/Selector.h"

#include "Offline/RecoDataProducts/inc/StrawHit.hh"
#include "Offline/RecoDataProducts/inc/ComboHit.hh"
#include "Offline/RecoDataProducts/inc/TrkStrawHitSeed.hh"

using namespace std;

//-----------------------------------------------------------------------------
void TAnaDump::printComboHit(const mu2e::ComboHit* Hit, const mu2e::StrawGasStep* Step, const char* Opt, int IHit, int Flags) {
  TString opt = Opt;
  opt.ToLower();

  if ((opt == "") || (opt.Index("banner") >= 0)) {
    printf("#---------------------------------------------------------------------------------------------------");
    printf("---------------------------------------------------------------------------------------------\n");
    printf("#   i nsh   sid   flags  stn pln pnl str      x        y       z      phi   time    tcorr     e(keV)");
    printf("  drtime  prtime  tres     wdist     wres simid      p        pz         pdg     pdgm   genid\n");
    printf("#---------------------------------------------------------------------------------------------------");
    printf("---------------------------------------------------------------------------------------------\n");
  }

  if (opt == "banner") return;

  const mu2e::SimParticle * sim (0);
    
  int      pdg_id(-1), mother_pdg_id(-1), generator_id(-1), sim_id(-1);
  double   mc_mom(-1.);
  double   mc_mom_z(-1.);

  mu2e::GenId gen_id;

  if (Step) {
    art::Ptr<mu2e::SimParticle> const& simptr = Step->simParticle(); 
    art::Ptr<mu2e::SimParticle> mother        = simptr;

    while(mother->hasParent()) mother = mother->parent();

    sim           = mother.operator ->();

    pdg_id        = simptr->pdgId();
    mother_pdg_id = sim->pdgId();

    if (simptr->fromGenerator()) generator_id = simptr->genParticle()->generatorId().id();
    else                         generator_id = -1;

    sim_id        = simptr->id().asInt();
    mc_mom        = Step->momvec().mag();
    mc_mom_z      = Step->momvec().z();
  }
    
  if ((opt == "") || (opt.Index("data") >= 0)) {
    if (IHit  >= 0) printf("%5i " ,IHit);
    else            printf("      ");

    printf("%3i ",Hit->nStrawHits());

    printf("%5u",Hit->strawId().asUint16());

    printf(" %08x",Flags);

    printf(" %3i %3i %3i %3i %8.2f %8.2f %8.2f %5.2f %8.2f %8.2f %8.4f %7.2f %7.2f %5.2f %9.3f %8.3f %5i %8.3f %8.3f %10i %10i %5i\n",
	   Hit->strawId().station(),
	   Hit->strawId().plane(),
	   Hit->strawId().panel(),
	   Hit->strawId().straw(),
	   Hit->pos().x(),Hit->pos().y(),Hit->pos().z(),
           Hit->pos().phi(),
	   Hit->time(),
	   Hit->correctedTime(),
	   Hit->energyDep()*1e3,

	   // (int) Hit->driftEnd(),
	   Hit->driftTime(),
	   Hit->propTime(),
	   Hit->transRes(),
	   Hit->wireDist(),
	   Hit->wireRes(),

	   sim_id,
	   mc_mom,
	   mc_mom_z,

	   pdg_id,
	   mother_pdg_id,
	   generator_id
           );
  }
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void TAnaDump::printComboHitCollection(const char* ComboHitCollTag   , 
				       const char* StrawDigiMCCollTag,
				       double TMin, double TMax,
                                       double EMin, double EMax) {
//-----------------------------------------------------------------------------
// get straw hits
//-----------------------------------------------------------------------------
  art::Handle<mu2e::ComboHitCollection> chcH;
  const mu2e::ComboHitCollection*       chc(nullptr);
  fEvent->getByLabel(ComboHitCollTag,chcH);

  printf("printing ComboHitCollection tag:%s\n",ComboHitCollTag);
  
  if (not chcH.isValid()) {
    printf("ERROR: cant find ComboHitCollection tag=%s, EXIT\n",ComboHitCollTag);
    print_sh_colls();
    return;
  }

  chc = chcH.product();
  art::InputTag sdmc_tag = StrawDigiMCCollTag;
  if (sdmc_tag == "") sdmc_tag = fSdmcCollTag;

  art::Handle<mu2e::StrawDigiMCCollection> mcdH;
  fEvent->getByLabel<mu2e::StrawDigiMCCollection>(sdmc_tag,mcdH);
  const mu2e::StrawDigiMCCollection*  mcdigis(nullptr);
  if (not mcdH.isValid()) {
    printf("WARNING: cant find StrawDigiMCCollection tag=%s\n",sdmc_tag.encode().data());
    print_sdmc_colls();
  }
  else {
    mcdigis          = mcdH.product();
  }

  int banner_printed = 0;
  int nhits          = chc->size();
  
  for (int i=0; i<nhits; ++i) {
    const mu2e::ComboHit* hit = &chc->at(i);
    int ind     = hit->index(0);

    const mu2e::StrawGasStep* step(nullptr);

    if (mcdigis) {
      const mu2e::StrawDigiMC*  sdmc = &mcdigis->at(ind);
      step = sdmc->earlyStrawGasStep().get();
    }
                                        // flags back to a separate coll
                                        // and back again to the hit payload
    int flags = *((int*) &hit->flag());
    if (banner_printed == 0) {
      printComboHit(hit, step, "banner");
      banner_printed = 1;
    }
    if ((hit->time() >= TMin) and (hit->time() <= TMax) and (hit->energyDep() >= EMin) and (hit->energyDep() <= EMax)) {
      printComboHit(hit, step, "data", i, flags);
    }
  }
 
}
