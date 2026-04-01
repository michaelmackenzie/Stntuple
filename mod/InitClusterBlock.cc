//-----------------------------------------------------------------------------
//  Apr 2013 P.Murat: initialization of the MU2E STNTUPLE cluster block
//
//-----------------------------------------------------------------------------
#include <cstdio>
#include "TROOT.h"
#include "TFolder.h"
#include "TLorentzVector.h"
#include <vector>

#include "Stntuple/obj/TStnDataBlock.hh"

#include "Stntuple/obj/TStnCluster.hh"
#include "Stntuple/obj/TStnClusterBlock.hh"

#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Event.h"

#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"

#include "Offline/CalorimeterGeom/inc/DiskCalorimeter.hh"

// #include "Offline/RecoDataProducts/inc/KalRepPtrCollection.hh"

// #include "Offline/RecoDataProducts/inc/TrackClusterMatch.hh"

#include "Offline/MCDataProducts/inc/CaloClusterMC.hh"
#include "Offline/MCDataProducts/inc/CaloEDepMC.hh"
#include "Offline/MCDataProducts/inc/CaloHitMC.hh"
#include "Offline/MCDataProducts/inc/CaloMCTruthAssns.hh"
#include "Offline/MCDataProducts/inc/GenParticle.hh"
#include "Offline/MCDataProducts/inc/SimParticle.hh"
#include "Offline/MCDataProducts/inc/StepPointMC.hh"

#include "Offline/RecoDataProducts/inc/StrawHit.hh"
#include "Offline/RecoDataProducts/inc/CaloHit.hh"
#include "Offline/RecoDataProducts/inc/CaloCluster.hh"

//-----------------------------------------------------------------------------
// assume that the collection name is set, so we could grab it from the event
//-----------------------------------------------------------------------------
int  StntupleInitMu2eClusterBlock(TStnDataBlock* Block, AbsEvent* Evt, int Mode) {

  constexpr int verbose(0);

  //  const char*               oname = {"MuratInitClusterBlock"};

//   int                           station, ntrk;
//   KalRep                        *krep;
//  double                        h1_fltlen, hn_fltlen, entlen, fitmom_err;
//   TStnTrack*                    track;
//   const mu2e::StepPointMC*      step;
  const mu2e::CaloClusterCollection*  list_of_clusters = nullptr;
  const mu2e::CaloClusterMCCollection* list_of_mc_clusters = nullptr;
  const mu2e::CaloClusterMCTruthAssn* mc_assns = nullptr;

  constexpr double kMinECrystal = 0.1; // count crystals above 100 KeV

  static char                calo_module_label   [100], calo_description   [100];
  static char                calo_mc_module_label[100], calo_mc_description[100];
  static char                trcl_module_label   [100], trcl_description   [100];

  TStnClusterBlock*         cb = (TStnClusterBlock*) Block;
  TStnCluster*              cluster;

  //  static int  first_entry(1);

  cb->Clear();

  //
  // "makeCaloCluster" would be the process name, "AlgoCLOSESTSeededByENERGY" - the description,
  //

  cb->GetModuleLabel("mu2e::CaloClusterCollection",calo_module_label);
  cb->GetDescription("mu2e::CaloClusterCollection",calo_description);

  cb->GetModuleLabel("mu2e::CaloClusterMCCollection",calo_mc_module_label);
  cb->GetDescription("mu2e::CaloClusterMCCollection",calo_mc_description);

  cb->GetModuleLabel("mu2e::TrackClusterMatchCollection",trcl_module_label);
  cb->GetDescription("mu2e::TrackClusterMatchCollection",trcl_description );

  art::Handle<mu2e::CaloClusterCollection> calo_cluster_handle;
  if (calo_description[0] == 0) Evt->getByLabel(calo_module_label,calo_cluster_handle);
  else                          Evt->getByLabel(calo_module_label,calo_description,calo_cluster_handle);
  list_of_clusters = (mu2e::CaloClusterCollection*) &(*calo_cluster_handle);

  // Retrieve the MC information, if available
  if(calo_mc_module_label[0] != '\0') { // non-empty string
    art::Handle<mu2e::CaloClusterMCCollection> calo_mc_cluster_handle;
    if (calo_mc_description[0] == '\0') Evt->getByLabel(calo_mc_module_label,calo_mc_cluster_handle);
    else                                Evt->getByLabel(calo_mc_module_label,calo_mc_description,calo_mc_cluster_handle);
    list_of_mc_clusters = (mu2e::CaloClusterMCCollection*) &(*calo_mc_cluster_handle);

    art::Handle<mu2e::CaloClusterMCTruthAssn> mc_assns_handle; // assume this is produced by the same MC module
    if (calo_mc_description[0] == '\0') Evt->getByLabel(calo_mc_module_label,mc_assns_handle);
    else                                Evt->getByLabel(calo_mc_module_label,calo_mc_description,mc_assns_handle);
    mc_assns = (mu2e::CaloClusterMCTruthAssn*) &(*mc_assns_handle);

    if(!list_of_mc_clusters || !mc_assns)
      printf("[InitClusterBlock::%s] No MC cluster collection or MC Assns\n", __func__);
    else {
    }
  }
  if(verbose > 0) printf("[InitClusterBlock::%s] Found %zu clusters, %zu MC clusters, and %zu Assns\n",
                         __func__, list_of_clusters->size(),
                         (list_of_mc_clusters) ? list_of_mc_clusters->size() : 0, (mc_assns) ? mc_assns->size() : 0);

  // List sorted by cluster energy
  std::vector<const mu2e::CaloCluster*> list_of_pcl;
  const mu2e::CaloCluster    *cl = nullptr;
  const mu2e::CaloClusterMC  *mc_cl = nullptr;

  // Loop through all input clusters, adding them to the local cluster list
  for (auto it = list_of_clusters->begin(); it != list_of_clusters->end(); it++) {
    cl = &(*it);
    list_of_pcl.push_back(cl);
  }
//-----------------------------------------------------------------------------
// sort list of pointers such that the most energetic cluster goes the first
//-----------------------------------------------------------------------------
  std::sort(list_of_pcl.begin(), list_of_pcl.end(),
	    [](const mu2e::CaloCluster*& lhs, const mu2e::CaloCluster*& rhs)
	      { return lhs->energyDep() > rhs->energyDep(); } );

//   art::Handle<mu2e::TrackClusterLink>  trk_cal_map;
//   if (trcl_module_label[0] != 0) {
//     if (trcl_description[0] != 0) Evt->getByLabel(trcl_module_label,trcl_description,trk_cal_map);
//     else                          Evt->getByLabel(trcl_module_label,trk_cal_map);
//   }

  // Retrieve the calo geometry
  art::ServiceHandle<mu2e::GeometryService> geom;

  const mu2e::Calorimeter* cal(NULL);

  if      (geom->hasElement<mu2e::Calorimeter>() ) {
    mu2e::GeomHandle<mu2e::Calorimeter> cc;
    cal = cc.get();
  }
  // else if (geom->hasElement<mu2e::VaneCalorimeter>() ) {
  //   mu2e::GeomHandle<mu2e::VaneCalorimeter> vc;
  //   cal = vc.get();
  // }
  else if (geom->hasElement<mu2e::DiskCalorimeter>() ) {
    mu2e::GeomHandle<mu2e::DiskCalorimeter> dc;
    cal = dc.get();
  }
//-----------------------------------------------------------------------------
// tracks are supposed to be already initialized
//-----------------------------------------------------------------------------
  const mu2e::CaloHit           *hit;
  const CLHEP::Hep3Vector       *pos;
  int                           id, ncl;

  constexpr double ROuterRing = 600.; // outer ring where cosmics deposit more energy

  double                        sume, sume2, sumy, sumx, sumy2, sumx2, sumxy; int qn;
  double                        e, e1(-1.), e2, emean, e2mean, trms, e9, e25, out_ring_e;
  double                        x_0, y_0, r, max_r;

  ncl = list_of_clusters->size();
  for (int i=0; i<ncl; i++) {
    cluster               = cb->NewCluster();
    cl                    = list_of_pcl.at(i);
    cluster->fCaloCluster = cl;
    cluster->fDiskID      = cl->diskID();
    cluster->fEnergy      = cl->energyDep();
    cluster->fTime        = cl->time();

    const mu2e::CaloHitPtrVector& list_of_crystals = cluster->fCaloCluster->caloHitsPtrVector();
    int nh = list_of_crystals.size();

    if(verbose > 1) printf("  Cluster %i: E = %.2f, T = %.1f, Disk = %i, N(crystals) = %i\n", i, cluster->fEnergy, cluster->fTime, cluster->fDiskID, nh);

    // If MC info is available, find the corresponding MC cluster
    if(list_of_mc_clusters && mc_assns) {
      mc_cl = nullptr;
      for (const auto& assn : *(mc_assns)) {
        const auto reco = assn.first;
        const auto mc   = assn.second;
        if(!reco || !mc) continue;
        if (&(*cl) == &(*reco)) {
          // found it
          mc_cl = &(*mc);
          break;
        }
      }
      if(!mc_cl) printf("[InitClusterBlock::%s] %i/%i/%i: No MC cluster found for cluster %i, N(clusters) = %i, N(MC clusters) = %zu\n",
                        __func__, Evt->run(), Evt->subRun(), Evt->event(),
                        i, ncl, list_of_mc_clusters->size());
    }

//-----------------------------------------------------------------------------
// print individual crystals in local vane coordinate system
// Y and Z
//-----------------------------------------------------------------------------
    qn     = 0;
    sume   = 0;
    sume2  = 0;
    sumx   = 0;
    sumy   = 0;
    sumx2  = 0;
    sumxy  = 0;
    sumy2  = 0;

    e2     = 0;

    trms       = 0.; // RMS of hit times
    e9         = 0.; // Energy in 3x3 around (and including) main crystal
    e25        = 0.; // Energy in 5x5 around (and including) main crystal
    out_ring_e = 0.; // Energy in cluster crystals > 600 mm
    x_0        = 0.; // X of highest energy hit
    y_0        = 0.; // Y of highest energy hit
    max_r      = 0.; // Largest distance between main crystal and other crystals

    // main crystal neighbors and next neighbors
    std::vector<int> neighbors, nneighbors;

    for (int ih=0; ih<nh; ih++) {
      hit = &(*list_of_crystals.at(ih));
      e   = hit->energyDep();
      id  = hit->crystalID();
      const mu2e::Crystal* cr = &cal->crystal(id);

      pos = &cr->localPosition();

      if (e > kMinECrystal) {
        ++qn;
        sume  += e;
        sume2 += e*e;
        sumx  += e*pos->x();
        sumy  += e*pos->y();
        sumx2 += e*pos->x()*pos->x();
        sumxy += e*pos->x()*pos->y();
        sumy2 += e*pos->y()*pos->y();
        trms  += std::pow(hit->time() - cluster->fTime, 2);
        r      = pos->perp();
        cluster->fCrystalEnergies[qn-1] = e;
        cluster->fCrystalTimes   [qn-1] = hit->time();
        cluster->fCrystalIDs     [qn-1] = id;

        if (ih<2) { // most energetic two crystals
          e2 += e;
          if (ih == 0) { // most energetic crystal
            e1 = e;
            x_0 = pos->x();
            y_0 = pos->y();
            neighbors = cr->neighbors();
            nneighbors = cr->nextNeighbors();
          }
        }

        // high radius hits
        if(r > ROuterRing) out_ring_e += e;

        // energy of direct neighbors of highest energy hit
        const bool is_neighbor = ih == 0 || std::find(neighbors.begin(), neighbors.end(),
                                                      hit->crystalID()) != neighbors.end();
        const bool is_nneighbor = std::find(nneighbors.begin(), nneighbors.end(),
                                            hit->crystalID()) != nneighbors.end();
        if(is_neighbor) e9  += e;
        if(is_nneighbor || is_neighbor) e25 += e;

        // maximum distance between a hit and the main hit
        if(ih > 0) {
          const double r = std::sqrt(std::pow(x_0 - pos->x(), 2) +
                                     std::pow(y_0 - pos->y(), 2));
          max_r = std::max(max_r, r);
        }
      }
    }

    emean  = sume/(qn+1.e-12);
    e2mean = sume2/(qn+1.e-12);

    double xmean, ymean, x2mean, xymean, y2mean, sigxx, sigyy, sigxy, phi;

    xmean  = sumx /(sume+1.e-12);
    ymean  = sumy /(sume+1.e-12);
    x2mean = sumx2/(sume+1.e-12);
    y2mean = sumy2/(sume+1.e-12);
    xymean = sumxy/(sume+1.e-12);

    sigxx  = x2mean-xmean*xmean;
    sigxy  = xymean-xmean*ymean;
    sigyy  = y2mean-ymean*ymean;

    cluster->fX         = cl->cog3Vector().x();
    cluster->fY         = cl->cog3Vector().y();
    cluster->fZ         = cl->cog3Vector().z();
    cluster->fIx1       = -1.; // cl->cogRow();
    cluster->fIx2       = -1.; // cl->cogColumn();

    cluster->fNCrystals = nh;
    cluster->fNCr1      = qn;
    cluster->fYMean     = ymean;
					// reuse fZ to store X
    cluster->fZMean     = xmean;
    cluster->fSigY      = sqrt(y2mean-ymean*ymean);
    cluster->fSigZ      = sqrt(x2mean-xmean*xmean);
    cluster->fSigR      = sqrt(y2mean+x2mean-ymean*ymean-xmean*xmean);
//-----------------------------------------------------------------------------
// make sure that nothing goes into an overflow
//_____________________________________________________________________________
    cluster->fFrE1      = (sume  > 0.) ? e1/(sume) : 0.;
    cluster->fFrE2      = (sume  > 0.) ? e2/(sume) : 0.;
    cluster->fSigE1     = (sume  > 0.) ? sqrt(qn*(e2mean-emean*emean))/(sume) : 0.;
    cluster->fSigE2     = (emean > 0.) ? sqrt(qn*(e2mean-emean*emean))/(emean) : 0.;
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
    cluster->fSigXX     = sigxx;
    cluster->fSigXY     = sigxy;
    cluster->fSigYY     = sigyy;

    phi = 0.5*atan2(2*sigxy,sigxx-sigyy);

//     if (sigxx < sigyy) {
//       phi = phi + M_PI/2.;
//       cluster->fSigXX = sigyy;
//       cluster->fSigYY = sigxx;
//     }

    cluster->fNx        = cos(phi);
    cluster->fNy        = sin(phi);

    cluster->fTimeRMS   = std::sqrt(trms);
    cluster->fMaxR      = max_r;
    cluster->fE9        = e9;
    cluster->fE25       = e25;
    cluster->fOutRingE  = out_ring_e;

    if(verbose > 1) printf("  T(RMS) = %5.3f, Max R = %5.1f, E9 = %5.1f, E25 = %5.1f, Out Ring E = %5.1f, R = %5.1f\n",
                           cluster->fTimeRMS, cluster->fMaxR,
                           cluster->fE9, cluster->fE25, cluster->fOutRingE,
                           std::sqrt(std::pow(cluster->fX, 2) + std::pow(cluster->fY, 2)));

//-----------------------------------------------------------------------------
// MC information
//_____________________________________________________________________________
    if(mc_cl) {
      // Map sim --> energy deposited
      std::map<art::Ptr<mu2e::SimParticle>, float> sim_edep;
      std::map<art::Ptr<mu2e::SimParticle>, float> sim_mom_in;
      float total_time(0.f), total_edep(mc_cl->totalEnergyDep());
      for(const auto& hit : mc_cl->caloHitMCs()) {
        if(verbose > 2) printf("    Hit: N(edeps) = %2zu\n", hit->energyDeposits().size());
        for(const auto& edep : hit->energyDeposits()) {
          const auto sim = edep.sim();
          const float energy = edep.energyDep();
          sim_edep[sim] += energy;
          sim_mom_in[sim] = std::max(sim_mom_in[sim], edep.momentumIn());
          total_time += edep.time() * energy / total_edep;
          if(verbose > 2) printf("      Edep: E = %6.2f, T = %6.1f, mom in = %6.2f, ID = %4lu\n",
                                 energy, edep.time(), edep.momentumIn(), sim->id().asInt());
        }
      }
      // Find the main sim info
      int main_sim(-1), pdg(0);
      float edep(-1.f), mom_in(-1.f);
      for(const auto& entry : sim_edep) {
        if(entry.second > edep) {
          main_sim = entry.first->id().asInt();
          pdg = entry.first->pdgId();
          edep = entry.second;
          mom_in = sim_mom_in[entry.first];
        }
      }
      cluster->fMCSimID   = main_sim;
      cluster->fMCSimPDG  = pdg;
      cluster->fMCSimEDep = edep;
      cluster->fMCSimMomIn = mom_in;
      cluster->fMCEDep    = total_edep;
      cluster->fMCTime    = total_time;
      if(verbose > 1) printf("  --> Associated MC cluster found: E = %6.2f, E(G4) = %6.2f, T = %6.1f, N(MC hits) = %2zu, ID = %4i, PDG = %5i, E(sim) = %6.2f, Mom in = %6.2f\n",
                             total_edep, mc_cl->totalEnergyDepG4(), total_time,
                             mc_cl->caloHitMCs().size(), main_sim, pdg, edep, mom_in);
    }

//     unsigned int nm = (*trk_cal_map).size();
//     for(size_t im=0; i<nm; im++) {
//       //	KalRepPtr const& trkPtr = fTrkCalMap->at(i).first->trk();
//       //	const KalRep *  const &trk = *trkPtr;

//       cl = &(*(*trk_cal_map).at(im).second);

//       if (cl == cluster->fCaloCluster) {
// 	cluster->fClosestTrack = fTrackBlock->Track(im);
// 	break;
//       }
//     }
  }
  return 0;
}

//_____________________________________________________________________________
Int_t StntupleInitMu2eClusterBlockLinks(TStnDataBlock* Block, AbsEvent* AnEvent, int Mode)
{
  // Mu2e version, do nothing

  Int_t  ev_number, rn_number;

  ev_number = AnEvent->event();
  rn_number = AnEvent->run();

  if (! Block->Initialized(ev_number,rn_number)) return -1;

					// do not do initialize links 2nd time

  if (Block->LinksInitialized()) return 0;

  TStnClusterBlock* header = (TStnClusterBlock*) Block;
  //  TStnEvent* ev   = header->GetEvent();
//-----------------------------------------------------------------------------
// mark links as initialized
//-----------------------------------------------------------------------------
  header->fLinksInitialized = 1;

  return 0;
}
