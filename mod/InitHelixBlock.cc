//-----------------------------------------------------------------------------
// Apr 2016 G. Pezzullo: initialization of the MU2E STNTUPLE Helix block
// Feb 2023 P.Murat: make initialization a class
//-----------------------------------------------------------------------------
#include <cstdio>
#include "TROOT.h"
#include "TFolder.h"
#include "TLorentzVector.h"
#include "TVector2.h"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"

#include "Stntuple/mod/InitHelixBlock.hh"

#include "Stntuple/obj/TStnDataBlock.hh"
#include "Stntuple/obj/TStnNode.hh"
#include "Stntuple/obj/TStnEvent.hh"

#include "Stntuple/obj/TStnTimeCluster.hh"
#include "Stntuple/obj/TStnTimeClusterBlock.hh"

#include "Stntuple/obj/TStnHelix.hh"
#include "Stntuple/obj/TStnHelixBlock.hh"

#include "Stntuple/obj/TStnTrackSeed.hh"
#include "Stntuple/obj/TStnTrackSeedBlock.hh"

#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Event.h"

#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"

#include "Offline/CalorimeterGeom/inc/Calorimeter.hh"
#include "Offline/TrackerGeom/inc/Tracker.hh"

#include "Offline/RecoDataProducts/inc/TimeCluster.hh"
#include "Offline/RecoDataProducts/inc/HelixSeed.hh"
#include "Offline/RecoDataProducts/inc/HelixHit.hh"
#include "Offline/RecoDataProducts/inc/KalSeed.hh"
#include "Offline/RecoDataProducts/inc/KalSeedAssns.hh"

#include "Offline/RecoDataProducts/inc/CaloCluster.hh"

#include "Offline/MCDataProducts/inc/StrawDigiMC.hh"
#include "Offline/MCDataProducts/inc/SimParticle.hh"
#include "Offline/MCDataProducts/inc/StepPointMC.hh"
#include "Offline/CommonMC/inc/TrkMCTools.hh"
#include "Offline/Mu2eUtilities/inc/HelixTool.hh"


using namespace ROOT::Math;
//-----------------------------------------------------------------------------
// assume that the collection name is set, so we could grab it from the event
//-----------------------------------------------------------------------------
int  StntupleInitHelixBlock::InitDataBlock(TStnDataBlock* Block, AbsEvent* Evt, int Mode) {

  const char* name = Form("InitHelixBlock::%s::%s", __func__, fHSeedCollTag.encode().c_str());

  //  mu2e::AlgorithmIDCollection*     aid_coll    (0);

  //   char                 helix_module_label[100], helix_description[100];
  // char                 makeSD_module_label[100];

  int ev_number = Evt->event();
  int rn_number = Evt->run();
  int sr_number = Evt->subRun();

  if (Block->Initialized(ev_number,rn_number,sr_number)) return 0;

  TStnHelixBlock*      cb = (TStnHelixBlock*) Block;
  TStnHelix*           helix(0);

  cb->Clear();
  //  cb->GetModuleLabel("mu2e::StrawDigiMCCollection",makeSD_module_label);
//-----------------------------------------------------------------------------
// algorithm ID's are determined by the same modules as the helices themselves
//-----------------------------------------------------------------------------
  art::Handle<mu2e::HelixSeedCollection>   hch;

  fListOfHSeeds = nullptr;
  if (not fHSeedCollTag.empty()) {
    Evt->getByLabel(fHSeedCollTag,hch);
    if (hch.isValid()) fListOfHSeeds = hch.product();
  }

  const mu2e::StrawDigiMCCollection* sdmcColl(nullptr);
  art::Handle<mu2e::StrawDigiMCCollection> sdmccH;
  if(Evt->getByLabel(fSdmcCollTag, sdmccH) && sdmccH.isValid()) {
    sdmcColl = sdmccH.product();
  } else {
    printf("InitHelixBlock::%s::%s: Did not find StrawDigiMC collection %s\n Available collections:\n",
           __func__, fHSeedCollTag.encode().c_str(), fSdmcCollTag.encode().c_str());
    auto handles = Evt->getMany<mu2e::StrawDigiMCCollection>();
    for(auto handle : handles) printf(" %s\n", handle.provenance()->productDescription().branchName().c_str());
  }

  const mu2e::HelixSeed     *tmpHel(0);
  const mu2e::RobustHelix   *robustHel(0);
  const mu2e::CaloCluster   *cluster(0);

  const int nhelices = (fListOfHSeeds) ? fListOfHSeeds->size() : 0;

  TParticlePDG* part(nullptr);
  TDatabasePDG* pdg_db = TDatabasePDG::Instance();

  static  CLHEP::Hep3Vector zaxis(0.0,0.0,1.0); // unit in z direction
  mu2e::GeomHandle<mu2e::Tracker> th;
  const mu2e::Tracker* trackerGeom = th.get();

  const int verbose(fVerbose);
  if(verbose > 3) printf("InitHelixBlock %s: Printing helix collection info: N(helices) = %2i\n", fHSeedCollTag.encode().c_str(), nhelices);
  for (int i=0; i<nhelices; i++) {
    std::vector<int>     hits_simp_id, hits_simp_index, hits_simp_z;

    helix                  = cb->NewHelix();
    tmpHel                 = &fListOfHSeeds->at(i);
    cluster                = tmpHel->caloCluster().get();
    robustHel              = &tmpHel->helix();
    if (cluster) {
      mu2e::GeomHandle<mu2e::Calorimeter> ch;
      const mu2e::Calorimeter* _calorimeter = ch.get();

      helix->fClusterTime    = cluster->time();
      helix->fClusterEnergy  = cluster->energyDep();
      CLHEP::Hep3Vector gpos = _calorimeter->diskToMu2e(cluster->diskID(),cluster->cog3Vector());
      CLHEP::Hep3Vector tpos = _calorimeter->mu2eToTracker(gpos);
      helix->fClusterX       = tpos.x();
      helix->fClusterY       = tpos.y();
      helix->fClusterZ       = tpos.z();
    } else {
      helix->fClusterTime    = 0.f;
      helix->fClusterEnergy  = 0.f;
      helix->fClusterX       = 0.f;
      helix->fClusterY       = 0.f;
      helix->fClusterZ       = 0.f;
    }

    helix->fHelix        = tmpHel;
    helix->fHelicity     = robustHel->helicity()._value;
    helix->fT0           = tmpHel->t0()._t0;
    helix->fT0Err        = tmpHel->t0()._t0err;
    helix->fRCent        = robustHel->rcent  ();
    helix->fFCent        = robustHel->fcent  ();
    helix->fRadius       = robustHel->radius ();
    helix->fLambda       = robustHel->lambda ();
    helix->fFZ0          = robustHel->fz0    ();
    helix->fD0           = robustHel->rcent  () - robustHel->radius ();

    const mu2e::HelixHitCollection* hits = &tmpHel->hits();
    int nhits = hits->size();

    const mu2e::ComboHit*           hit(0);
//-----------------------------------------------------------------------------
// figure out which algorithm was used to reconstruct the helix - helix collections
// could be merged
// 1: RobustHelixFinder 2: CalHelixFinder
//-----------------------------------------------------------------------------
    if (tmpHel->status().hasAnyProperty(mu2e::TrkFitFlagDetail::CPRHelix)) {
      helix->fAlgorithmID = 0x20002;
    }
    else {
      helix->fAlgorithmID = 0x10001;
    }

    int      nStrawHits(0), nLowPHits(0);
    float    first_hit_z(0), last_hit_z(0);

    if(verbose > 5) printf("InitHelixBlock %s: Printing helix %i hit info:\n", Block->GetTitle(), i);
    for (int j=0; j<nhits; ++j) {      //this is a loop over the ComboHits
      hit       = &hits->at(j);

      if(j==0) first_hit_z = hit->pos().z();
      else if(j==nhits-1) last_hit_z = hit->pos().z();

      // get the MC truth info
      if (hit->_flag.hasAnyProperty(mu2e::StrawHitFlag::outlier))         continue;

      if(sdmcColl) {
        std::vector<StrawDigiIndex> shids;
        tmpHel->hits().fillStrawDigiIndices(j,shids);

        if(verbose > 5) printf(" %2i: Idx   ID  PDGID  SimID Parent   Z     T      P(MC)\n", j);
        const float minP(10.);//MeV/c, minimum MC sim P to count in the hits
        for (size_t k=0; k<shids.size(); ++k) {
          const mu2e::StrawDigiMC* sdmc = &sdmcColl->at(shids[k]);
          auto const& spmcp = sdmc->earlyStrawGasStep();
          art::Ptr<mu2e::SimParticle> const& simptr = spmcp->simParticle();
          int sim_id        = simptr->id().asInt();
          float   dz        = spmcp->position().z();// - trackerZ0;
          float   pMC       = std::sqrt(spmcp->momentum().mag2());
          if (verbose > 5) printf("     %2li %5i %5i %5i %5li %8.1f %7.1f %6.1f\n",
                                  k, shids[k], simptr->pdgId(), sim_id, simptr->parentId().asInt(), dz, spmcp->time(), pMC);
          if (pMC<minP) {
            ++nLowPHits;
            continue;
          }
          hits_simp_id.push_back   (sim_id);
          hits_simp_index.push_back(shids[k]);
          hits_simp_z.push_back(dz);
          break;
        }
      }
      nStrawHits += hit->nStrawHits();
    }

//-----------------------------------------------------------------------------
// calculate the number of loops made
//-----------------------------------------------------------------------------
    helix->fNLoops = (last_hit_z - first_hit_z)/(fabs(helix->fLambda)*2.*M_PI);

//-----------------------------------------------------------------------------
// evaluate the following Helix paramters: TZSlope, TZSlope error, TZChi2NDof
// and the hitRatio (expected/measured)
//-----------------------------------------------------------------------------
    mu2e::HelixTool helTool(tmpHel, trackerGeom);
    float           hSlope(0), hSlopeError(0), chi2ndof(0);
    helTool.dirOfProp(hSlope, hSlopeError, chi2ndof);

    helix->fTZSlope      = hSlope;
    helix->fTZSlopeError = hSlopeError;
    helix->fChi2TZNDof   = chi2ndof;
    helix->fHitRatio     = helTool.hitRatio();
    helix->fPropDir      = tmpHel->propDir();

    if(verbose > 3) printf(" Helix %2i: P = %.1f, Pt = %.1f, Pz = %.1f, R = %.1f, T = %.1f, N(hits) = %3i, slope = %.2e +- %.2e\n", i,
                           helix->P(), helix->Pt(), helix->Pz(), helix->Radius(), helix->T0(), nStrawHits, helix->fTZSlope, helix->fTZSlopeError);

//-----------------------------------------------------------------------------
// find the SimParticle that created the majority of the hits
//-----------------------------------------------------------------------------
    int     max(0), mostvalueindex(-1), mostvalue(-1), id_max(0);
    float   dz_most(1e4);

    for (int  k=0; k<(int)hits_simp_id.size(); ++k){
      int co = (int)std::count(hits_simp_id.begin(), hits_simp_id.end(), hits_simp_id[k]);
      if ( (co>0) && (co>max)) {
        float  dz      = std::fabs(hits_simp_z[k]);
        if (dz < dz_most){
          dz_most        = dz;
          max            = co;
          id_max         = k;
          mostvalue      = hits_simp_id[k];
          mostvalueindex = hits_simp_index[k];
        }
      }
    }

    helix->fSimpId1Hits = max;
    helix->fSimpId2Hits = -1;
    helix->fSimpPDGM1   = -1;
    helix->fSimpPDGM2   = -1;
    helix->fSimpID1     = mostvalue;

    if (hits_simp_id.size()>0) {
      if ( (mostvalueindex != hits_simp_index[id_max]) ){
        printf(">>> %s::%i: ERROR: event %i helix %i no MC Sim Particle found. MostValueindex = %i hits_simp_index[id_max] = %i\n",
               name, __LINE__, Evt->event(), i, mostvalueindex, hits_simp_index[id_max]);
      }
    }
    const bool has_best_simp = sdmcColl && !((mostvalueindex<0) || (mostvalueindex >= (int) sdmcColl->size()));
    if(!has_best_simp) {
      if(sdmcColl && nLowPHits == 0) {
        printf(">>> %s::%i: ERROR: event %i helix %i no MC found. MostValueindex = %i mcdigis_size = %li size(hits_simp_index) = %lu\n",
               name, __LINE__, Evt->event(), i, mostvalueindex, sdmcColl->size(), hits_simp_index.size());
        if (hits_simp_index.size()>0) {
          printf(">>> %s::%i: ERROR:  hits_simp_index[id_max] = %i  \n",
                 name, __LINE__, hits_simp_index[id_max]);
        }
      }
    } else {

      const mu2e::StrawDigiMC* sdmc = &sdmcColl->at(mostvalueindex);

      auto const& step = sdmc->earlyStrawGasStep();

      const mu2e::SimParticle* sim(nullptr);

      art::Ptr<mu2e::SimParticle> const& simptr = step->simParticle();
      helix->fSimpPDG1    = simptr->pdgId();
      art::Ptr<mu2e::SimParticle> mother = simptr;
      part   = pdg_db->GetParticle(helix->fSimpPDG1);

      while(mother->hasParent()) mother = mother->parent();
      sim = mother.operator ->();

      helix->fSimpPDGM1   = sim->pdgId();

      double   px = step->momentum().x();
      double   py = step->momentum().y();
      double   pz = step->momentum().z();
      double   mass  (-1.);
      double   energy(-1.);
      if (part) {
        mass   = part->Mass();
        energy = sqrt(px*px+py*py+pz*pz+mass*mass);
      }
      helix->fSimpMom1.SetPxPyPzE(px,py,pz,energy);

      CLHEP::Hep3Vector sp = simptr->startPosition();
      helix->fSimpOrigin1.SetXYZT(sp.x(),sp.y(),sp.z(),simptr->startGlobalTime());

      if(verbose > 5 || (verbose > 1 && energy < 70.f && helix->P() > 85.f)) printf(" Simp1: PDG = %5i; (pT, pZ, p, E) = (%.1f, %.1f, %.1f %.1f); (x0, y0, r0, z0, t0) = (%.1f, %.1f, %.1f, %.1f, %.1f), N(hits) = %2i\n",
                                                                                    helix->fSimpPDG1, helix->fSimpMom1.Pt(), helix->fSimpMom1.Pz(), helix->fSimpMom1.P(), helix->fSimpMom1.E(),
                                                                                    helix->fSimpOrigin1.X(), helix->fSimpOrigin1.Y(), std::sqrt(std::pow(helix->fSimpOrigin1.X()+3904.f,2) + std::pow(helix->fSimpOrigin1.Y(),2)),
                                                                                    helix->fSimpOrigin1.Z(), helix->fSimpOrigin1.T(), helix->fSimpId1Hits);

      //look for the second most frequent hit
      if (max != int(hits_simp_id.size()) ){  //nhits){
        int   secondmostvalueindex(-1);
        max     = 0;//reset max
        dz_most = 1e4;

        for (int k=0; k<(int)hits_simp_id.size(); ++k){
          int value = hits_simp_id[k];
          int co = (int)std::count(hits_simp_id.begin(), hits_simp_id.end(), value);
          if ( (co>0) && (co>=max) && (value != mostvalue)) {
            float  dz      = std::fabs(hits_simp_z[k]);
            if (dz < dz_most){
              max                  = co;
              dz_most              = dz;
              secondmostvalueindex = hits_simp_index[k];
            }
          }
        }
        helix->fSimpId2Hits = max;

        if (secondmostvalueindex >= 0) {

          const mu2e::StrawGasStep* step(nullptr);

          const mu2e::StrawDigiMC*  sdmc = &sdmcColl->at(secondmostvalueindex);
          if (sdmc->wireEndTime(mu2e::StrawEnd::cal) < sdmc->wireEndTime(mu2e::StrawEnd::hv)) {
            step = sdmc->strawGasStep(mu2e::StrawEnd::cal).get();
          }
          else {
            step = sdmc->strawGasStep(mu2e::StrawEnd::hv ).get();
          }

          const mu2e::SimParticle* sim (nullptr);

          if (step) {
            art::Ptr<mu2e::SimParticle> const& simptr = step->simParticle();
            helix->fSimpPDG2    = simptr->pdgId();
            art::Ptr<mu2e::SimParticle> mother = simptr;
            part   = pdg_db->GetParticle(helix->fSimpPDG2);

            while(mother->hasParent()) mother = mother->parent();
            sim = mother.operator ->();

            helix->fSimpPDGM2   = sim->pdgId();

            double   px = simptr->startMomentum().x();
            double   py = simptr->startMomentum().y();
            double   pz = simptr->startMomentum().z();

            double   mass  (-1.), energy(-1.);
            if (part) {
              mass   = part->Mass();
              energy = sqrt(px*px+py*py+pz*pz+mass*mass);
            }
            helix->fSimpMom2.SetPxPyPzE(px,py,pz,energy);

            const CLHEP::Hep3Vector sp = simptr->startPosition();
            helix->fSimpOrigin2.SetXYZT(sp.x(),sp.y(),sp.z(),simptr->startGlobalTime());
          }
        }
      }
    }

    helix->fNComboHits   = tmpHel->hits().size();
    helix->fNHits        = nStrawHits;
    helix->fChi2XYNDof   = robustHel->chi2dXY();
    helix->fChi2PhiZNDof = robustHel->chi2dZPhi();
  }
//-----------------------------------------------------------------------------
// on return set event and run numbers to mark block as initialized
//-----------------------------------------------------------------------------
  cb->f_RunNumber    = rn_number;
  cb->f_EventNumber  = ev_number;
  cb->f_SubrunNumber = sr_number;

  return 0;
}

//-----------------------------------------------------------------------------
Int_t StntupleInitHelixBlock::ResolveLinks(TStnDataBlock* Block, AbsEvent* AnEvent, int Mode)
{

  Int_t  evt, run, srn;

  evt = AnEvent->event();
  run = AnEvent->run();
  srn = AnEvent->subRun();

  if (! Block->Initialized(evt,run,srn)) return -1;

                                        // do not do initialize links 2nd time

  if (Block->LinksInitialized()) return 0;
//-----------------------------------------------------------------------------
// for BTRK   fits (mode=1) this is trackseed <--> helix associations
// for KinKal fits (mode=2) this is track     <--> helix associations
//-----------------------------------------------------------------------------
  art::Handle<mu2e::KalHelixAssns> ksfhaH;
  const mu2e::KalHelixAssns* ksfha(0);
  AnEvent->getByLabel(fKsCollTag, ksfhaH);
  if (ksfhaH.isValid()) ksfha = ksfhaH.product();

  TStnEvent*      ev = Block->GetEvent();
  TStnHelixBlock* hb = (TStnHelixBlock*) Block;
  int             nh(0);
  if (ksfha) nh = hb->NHelices();
//-----------------------------------------------------------------------------
// this is a hack, to be fixed soon
//-----------------------------------------------------------------------------
  int ntseeds(0), ntpeaks(0);

  TStnTrackSeedBlock*   tsb = (TStnTrackSeedBlock*  ) ev->GetDataBlock(fKsfBlockName.Data());
  TStnTimeClusterBlock* tcb = (TStnTimeClusterBlock*) ev->GetDataBlock(fTclBlockName.Data());

  if (tsb) ntseeds = tsb->NTrackSeeds();
  if (tcb) ntpeaks = tcb->NTimeClusters();

  for (int i=0; i<nh; i++) {
    TStnHelix* hel = hb->Helix(i);
    const mu2e::HelixSeed* hs1 = hel->fHelix;
//-----------------------------------------------------------------------------
// looking for the seed in associations - a helix may not have a seed
//-----------------------------------------------------------------------------
    const mu2e::KalSeed* ksf(nullptr);
    if (ksfha){
      for (auto ass: *ksfha) {
        const mu2e::HelixSeed* hs2 = ass.second.get();
        if (hs1 == hs2) {
          ksf = ass.first.get();
          break;
        }
      }
    }
//-----------------------------------------------------------------------------
// if I knew the collection tag, I could use that instead
//-----------------------------------------------------------------------------
    const mu2e::KalSeedCollection* kscoll(nullptr);

    if (not fKsCollTag.empty()) {
      art::Handle<mu2e::KalSeedCollection> kscoll_h;
      AnEvent->getByLabel(fKsCollTag,kscoll_h);
      if (kscoll_h.isValid()) kscoll = kscoll_h.product();
    }

//-----------------------------------------------------------------------------
// if everything is consistent, kscoll has nkseeds elements in it
//-----------------------------------------------------------------------------
    int ksfIndex(-1);

    if (kscoll != nullptr) {
      for (int j=0; j<ntseeds; ++j){
        const mu2e::KalSeed* ks2 = &kscoll->at(j);
        if (ks2 == ksf) {
          ksfIndex = j;
          break;
        }
      }
    }

    hel->SetTrackSeedIndex(ksfIndex);

    const mu2e::TimeCluster* ktimepeak = hs1->timeCluster().get();

    int      tclIndex(-1);
    for (int j=0; j<ntpeaks;++j){
      TStnTimeCluster* tp = tcb->TimeCluster(j);
      const mu2e::TimeCluster* fktimepeak = tp->fTimeCluster;
      if (fktimepeak == ktimepeak){
        tclIndex = j;
        break;
      }
    }

    hel->SetTimeClusterIndex(tclIndex);
  }
//-----------------------------------------------------------------------------
// mark links as initialized
//-----------------------------------------------------------------------------
  hb->fLinksInitialized = 1;

  return 0;
}
