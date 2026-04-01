///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

#include "Stntuple/mod/InitCrvClusterBlock.hh"
#include "Offline/RecoDataProducts/inc/CrvRecoPulse.hh"
// #include "Offline/RecoDataProducts/inc/CrvCoincidence.hh"
#include "Offline/RecoDataProducts/inc/CrvCoincidenceCluster.hh"
#include "Offline/MCDataProducts/inc/CrvCoincidenceClusterMC.hh"
#include "Offline/MCDataProducts/inc/CrvCoincidenceClusterMCAssns.hh"

//-----------------------------------------------------------------------------
// in this case AbsEvent is just not used
//-----------------------------------------------------------------------------
int StntupleInitCrvClusterBlock::InitDataBlock(TStnDataBlock* Block, AbsEvent* Event, int Mode) {

  const int ev = Event->event();
  const int rn = Event->run();
  const int sr = Event->subRun();

  if (Block->Initialized(ev,rn,sr)) return 0;

  const int verbose(0);

  TCrvClusterBlock* block = (TCrvClusterBlock*) Block;

  block->f_EventNumber  = ev;
  block->f_RunNumber    = rn;
  block->f_SubrunNumber = sr;
//-----------------------------------------------------------------------------
// initialize pointer to the pulse collection
//-----------------------------------------------------------------------------
  art::Handle<mu2e::CrvRecoPulseCollection> crpch;
  const mu2e::CrvRecoPulseCollection*       crpc(nullptr);
  int   ncrp(0);

  if(!fCrvRecoPulseCollTag.empty()) {
    if (Event->getByLabel(fCrvRecoPulseCollTag,crpch)) {
      crpc = crpch.product();
      ncrp = crpc->size();
    } else {
      printf("InitCrvClusterBlock::%s: No CRV pulse collection (%s) found\n", __func__, fCrvRecoPulseCollTag.encode().c_str());
    }
  }

  const mu2e::CrvRecoPulse* p0 = (ncrp > 0) ? &crpc->at(0) : nullptr;
//-----------------------------------------------------------------------------
// store CrvCoincidenceCluster's
//-----------------------------------------------------------------------------
  art::Handle<mu2e::CrvCoincidenceClusterCollection> cccch;
  const mu2e::CrvCoincidenceClusterCollection*       cccc(nullptr);
  int                                                nccc(0);

  if (!fCrvCoincidenceClusterCollTag.empty()) {
    if (Event->getByLabel(fCrvCoincidenceClusterCollTag,cccch)) {
      cccc = cccch.product();
      nccc = cccc->size();
    } else {
      printf("InitCrvClusterBlock::%s: No CRV coincidence cluster collection (%s) found\n", __func__, fCrvCoincidenceClusterCollTag.encode().c_str());
    }
  }

  // Retrieve MC information if it's available
  art::Handle<mu2e::CrvCoincidenceClusterMCAssns> mc_ccc_assnsH;
  const mu2e::CrvCoincidenceClusterMCAssns*       mc_ccc_assns(nullptr);
  if(!fCrvCoincidenceClusterMCCollTag.empty()) {
    if(Event->getByLabel(fCrvCoincidenceClusterMCCollTag,mc_ccc_assnsH)) mc_ccc_assns = mc_ccc_assnsH.product();
    else printf("InitCrvClusterBlock::%s: No MC <--> Reco CRV coincidence cluster associations (%s) found\n", __func__, fCrvCoincidenceClusterMCCollTag.encode().c_str());
  }
  const int nmc_ccc_assns = (mc_ccc_assns) ? mc_ccc_assns->size() : 0;
  if(mc_ccc_assns && nmc_ccc_assns != nccc) printf("InitCrvClusterBlock::%s: MC cluster associations (%i) and Reco clusters (%i) don't match\n",
                                                   __func__, nmc_ccc_assns, nccc);

  //------------------------------------------------------
  // Loop through the clusters
  for (int iccc=0; iccc<nccc; iccc++) {
    if(verbose > 0) printf("InitCrvClusterBlock::%s: Processing cluster %i\n", __func__, iccc);
    const mu2e::CrvCoincidenceCluster* cluster = &cccc->at(iccc);
    const mu2e::CrvCoincidenceClusterMC* mc_cluster = nullptr;
    if(!cluster) {
      printf("InitCrvClusterBlock::%s: Cluster %i is not defined!\n", __func__, iccc);
      continue;
    }
    if(mc_ccc_assns) {
      for(int iassn = 0; iassn < nmc_ccc_assns; ++iassn) {
        const auto assn = mc_ccc_assns->at(iassn);
        if(!assn.second) {
          printf("InitCrvClusterBlock::%s: Cluster %2i: MC association %i is not valid! Reco = %o, MC = %o\n", __func__, iccc, iassn,
                 assn.first.isAvailable(), assn.second.isAvailable());
          continue;
        }
        const mu2e::CrvCoincidenceCluster*   ireco = (!assn.first) ? nullptr : &(*(assn.first));
        const mu2e::CrvCoincidenceClusterMC* imc   = &(*(assn.second));
        if(ireco && &(*(ireco)) == &(*cluster)) {
          if(verbose > 1) printf(" --> Associated MC cluster found, association index %i\n", iassn);
          mc_cluster = imc;
          break;
        }
      }
      if(!mc_cluster && nmc_ccc_assns == nccc) {
        // try by index if the numbers match
        const mu2e::CrvCoincidenceClusterMC* imc = &(*(mc_ccc_assns->at(iccc).second));
        mc_cluster = imc;
        if(verbose > 0) printf("%s:%s: Cluster %2i: Associated MC cluster found by index %i\n",
                               typeid(*this).name(), __func__, iccc, iccc);
      }
      if(!mc_cluster) printf("%s::%s: Cluster %2i: Associated MC cluster not found! N(clusters) = %i N(Assns) = %i\n",
                        typeid(*this).name(), __func__, iccc, nccc, nmc_ccc_assns);
    }

    TCrvCoincidenceCluster* ccc = block->NewCluster();

    const std::vector<art::Ptr<mu2e::CrvRecoPulse>>* list_of_pulses = &cluster->GetCrvRecoPulses();

    const int    sector = cluster->GetCrvSectorType();
    const int    np     = list_of_pulses->size();
    const int    npe    = cluster->GetPEs();
    const float  slope  = cluster->GetSlope();

    const double x      = cluster->GetAvgHitPos().x();
    const double y      = cluster->GetAvgHitPos().y();
    const double z      = cluster->GetAvgHitPos().z();
    const float  t1     = cluster->GetStartTime();
    const float  t2     = cluster->GetEndTime();

    ccc->Set(iccc,sector,np,npe,x,y,z,t1,t2, slope);
    if(verbose > 1) printf("  Cluster sector = %2i, N(pulses) = %2i, N(PE) = %3i, x = %7.1f, y = %7.1f, z = %8.1f, t1 = %6.1f, t2 = %6.1f, slope = %6.2f, mc_found = %o\n",
                           sector, np, npe, x, y, z, t1, t2, slope, mc_cluster != nullptr);
    if(mc_cluster) {
      auto sim = mc_cluster->GetMostLikelySimParticle();
      const int   sim_id  = (sim) ? sim->id().asInt() : -1;
      const int   mc_np   = mc_cluster->GetPulses().size();

      const float mc_edep = mc_cluster->GetTotalEnergyDeposited();
      const float mc_time = mc_cluster->GetAvgHitTime();
      const float mc_x    = mc_cluster->GetAvgHitPos().x();
      const float mc_y    = mc_cluster->GetAvgHitPos().y();
      const float mc_z    = mc_cluster->GetAvgHitPos().z();

      ccc->SetMC(sim_id, mc_np, mc_edep, mc_time, mc_x, mc_y, mc_z);
      if(verbose > 1) printf("  MC Info: SIM ID = %i, N(pulses) = %2i, E(dep) = %4.1f, x = %7.1f, y = %7.1f, z = %8.1f, tavg = %6.1f\n",
                             sim_id, mc_np, mc_edep, mc_x, mc_y, mc_z, mc_time);
    }
//-----------------------------------------------------------------------------
// now store pulses associated with the cluster
//-----------------------------------------------------------------------------
    if(fStorePulses) {
      for (int ip=0; ip<np; ip++) {
        const mu2e::CrvRecoPulse* pulse = list_of_pulses->at(ip).get();
        const int index = pulse-p0;
//-----------------------------------------------------------------------------
// check if the pulse is already stored
//-----------------------------------------------------------------------------
        int loc(-1);
        const int npulses = block->NPulses();

        for (int i=0; i<npulses; i++) {
          TCrvRecoPulse* p = block->Pulse(i);
          if (p->Index() == index) {
            loc   = i;
            break;
          }
        }
        if (loc == -1) {
//-----------------------------------------------------------------------------
// add pulse to the list
//-----------------------------------------------------------------------------
          loc                  = npulses;

          TCrvRecoPulse* new_pulse = block->NewPulse();

          const int   npes           = pulse->GetPEs();
          const int   npes_height    = pulse->GetPEsPulseHeight();
          const int   nind           = pulse->GetWaveformIndices().size();
          const int   bar            = pulse->GetScintillatorBarIndex().asInt();
          const int   sipm           = pulse->GetSiPMNumber();

          const float time           = pulse->GetPulseTime();
          const float height         = pulse->GetPulseHeight();
          const float width          = pulse->GetPulseBeta(); // was GetPulseWidth();
          const float chi2           = pulse->GetPulseFitChi2();
          const float le_time        = pulse->GetLEtime();

          new_pulse->Set(index,npes,npes_height,nind,bar,sipm,time,height,width,chi2,le_time);
        }
        block->fClusterPulseLinks->Add(iccc,loc);
      }
    }
  }
  if(verbose > 2) block->Print();
  return 0;
}


//-----------------------------------------------------------------------------
// keep this function as an example, don't really need it
//-----------------------------------------------------------------------------
int StntupleInitCrvClusterBlock::ResolveLinks(TStnDataBlock* Block, AbsEvent* Event, int Mode) {

  const int ev = Event->event();
  const int rn = Event->run();
  const int sr = Event->subRun();

  if (! Block->Initialized(ev,rn,sr)) return -1;

  return 0;
}
