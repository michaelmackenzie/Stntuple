//-----------------------------------------------------------------------------
//  Dec 26 2000 P.Murat: initialization of the STNTUPLE track block
//  2014-06-23: remove vane support
//-----------------------------------------------------------------------------
#include <cstdio>
#include <algorithm>
#include "TROOT.h"
#include "TFolder.h"
#include "TLorentzVector.h"
#include "TVector2.h"
					// Mu2e
#include "Stntuple/obj/TStnTrack.hh"
#include "Stntuple/obj/TStnTrackBlock.hh"
#include "Stntuple/obj/TStnEvent.hh"
#include "Stntuple/obj/TStnHelix.hh"
#include "Stntuple/obj/TStnHelixBlock.hh"

#include "Stntuple/obj/TStnTrackSeed.hh"
#include "Stntuple/obj/TStnTrackSeedBlock.hh"

#include "messagefacility/MessageLogger/MessageLogger.h"

#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/Handle.h"

#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"
#include "Offline/GeometryService/inc/VirtualDetector.hh"
#include "Offline/GeometryService/inc/DetectorSystem.hh"

#include "Offline/TrackerGeom/inc/Tracker.hh"
#include "Offline/CalorimeterGeom/inc/Calorimeter.hh"
#include "Offline/CalorimeterGeom/inc/DiskCalorimeter.hh"

#include "Offline/RecoDataProducts/inc/HelixSeed.hh"
#include "Offline/RecoDataProducts/inc/KalSeed.hh"
#include "Offline/RecoDataProducts/inc/KalSeedAssns.hh"
#include "Offline/RecoDataProducts/inc/KalSeed.hh"
#include "Offline/RecoDataProducts/inc/TrkFitFlag.hh"



// #include "Offline/BTrkData/inc/Doublet.hh"

#include "Offline/RecoDataProducts/inc/TrkStraw.hh"
// #include "Offline/RecoDataProducts/inc/TrkCaloIntersect.hh"
// #include "Offline/RecoDataProducts/inc/TrackClusterMatch.hh"

#include "Offline/MCDataProducts/inc/GenParticle.hh"
#include "Offline/MCDataProducts/inc/SimParticle.hh"
#include "Offline/MCDataProducts/inc/StrawGasStep.hh"
#include "Offline/DataProducts/inc/VirtualDetectorId.hh"
#include "Offline/DataProducts/inc/SurfaceId.hh"

#include "Offline/RecoDataProducts/inc/StrawDigi.hh"
#include "Offline/RecoDataProducts/inc/StrawHit.hh"
#include "Offline/RecoDataProducts/inc/StrawHitFlag.hh"
#include "Offline/RecoDataProducts/inc/CaloHit.hh"
#include "Offline/RecoDataProducts/inc/CaloCluster.hh"
#include "Offline/RecoDataProducts/inc/AlgorithmID.hh"

					          // BaBar
// #include "BTrk/ProbTools/ChisqConsistency.hh"
// #include "BTrk/BbrGeom/BbrVectorErr.hh"
// #include "BTrk/BbrGeom/HepPoint.h"
// #include "BTrk/BbrGeom/TrkLineTraj.hh"
// #include "BTrk/TrkBase/TrkPoca.hh"
// #include "BTrk/KalmanTrack/KalHit.hh"
#include "Stntuple/gui/HepPoint.hh"

#include "Stntuple/mod/InitTrackBlock.hh"
// #include "Stntuple/mod/THistModule.hh"
// #include "Stntuple/base/TNamedHandle.hh"

//-----------------------------------------------------------------------------
// Map[iplane][ipanel][il]: index of the layer in Z-ordered sequence
//-----------------------------------------------------------------------------
void StntupleInitTrackBlock::InitTrackerZMap(const mu2e::Tracker* Tracker, ZMap_t* Map) {
  int      ix, loc;
  double   z0, z1;

  int nplanes = Tracker->nPlanes();

  for (int ipl=0; ipl<nplanes; ipl++) {
    for (int isec=0; isec<6; isec++) {
      ix  = isec % 2;
      loc = 2*ipl+ix;
      Map->fMap[ipl][isec][0] = loc;
      Map->fMap[ipl][isec][1] = loc;
    }
    // form the list of Z-coordinates
    const mu2e::Straw *s0, *s1;

    s0 = &Tracker->getPlane(ipl).getPanel(0).getStraw(0);
    s1 = &Tracker->getPlane(ipl).getPanel(0).getStraw(1);
    z0 = s0->getMidPoint().z();
    z1 = s1->getMidPoint().z();

    Map->fZ[2*ipl] = (z0+z1)/2.;

    s0 = &Tracker->getPlane(ipl).getPanel(1).getStraw(0);
    s1 = &Tracker->getPlane(ipl).getPanel(1).getStraw(1);
    z0 = s0->getMidPoint().z();
    z1 = s1->getMidPoint().z();

    Map->fZ[2*ipl+1] = (z0+z1)/2.;
  }
}

//-----------------------------------------------------------------------------
// for a given Z find closest Z-layer, returns 'Plane'
// 'Offset' is a 'face number' within the Plane
//-----------------------------------------------------------------------------
void StntupleInitTrackBlock::get_station(const mu2e::Tracker* Tracker, ZMap_t* Map, double Z, int* Plane, int* Offset) {

  double dz, dz_min(1.e10);
  int    iface(-1);
  // looks that Device == Plane
  int nplanes = Tracker->nPlanes();
  // a plane has 2 "faces", 2 layers in each
  int nfaces  = 2*nplanes;

  for (int i=0; i<nfaces; i++) {
    dz = Map->fZ[i]-Z;
    if (fabs(dz) < dz_min) {
      iface  = i;
      dz_min = fabs(dz);
    }
  }

  *Plane   = iface / 2;
  *Offset  = iface % 2;
}

//-----------------------------------------------------------------------------
// extrapolate track to a given Z
//-----------------------------------------------------------------------------
double StntupleInitTrackBlock::s_at_given_z(const mu2e::KalSeed* KSeed, double Z) {

  printf("ERROR in StntupleInitTrackBlock:::s_at_given_z : REIMPLEMENT! \n");
  return -1;

  // double  ds(10.), s0, s1, s2, z0, z1, z2, dzds, sz, sz1, z01;

  // // s1     = Kffs->firstHit()->kalHit()->hit()->fltLen();
  // // s2     = Kffs->lastHit ()->kalHit()->hit()->fltLen();

  // const TrkHitVector* hots = &Kffs->hitVector();
  // int nh = hots->size();

  // const TrkHit *first(nullptr), *last(nullptr);

  // for (int ih=0; ih<nh; ++ih) {
  //   const TrkHit* hit = hots->at(ih);
  //   if (hit  != nullptr) {
  //     if (first == nullptr) first = hit;
  //     last = hit;
  //   }
  // }

  // s1 = first->fltLen();
  // s2 = last ->fltLen();

  // z1     = Kffs->position(s1).z();
  // z2     = Kffs->position(s2).z();

  // dzds   = (z2-z1)/(s2-s1);
  // //-----------------------------------------------------------------------------
  // // iterate once, choose the closest point
  // //-----------------------------------------------------------------------------
  // if (fabs(Z-z1) > fabs(Z-z2)) {
  //   z0 = z2;
  //   s0 = s2;
  // }
  // else {
  //   z0 = z1;
  //   s0 = s1;
  // }

  // sz    = s0+(Z-z0)/dzds;

  // z0     = Kffs->position(sz).z();     // z0 has to be close to Z(TT_FrontPA)
  // z01    = Kffs->position(sz+ds).z();

  // dzds   = (z01-z0)/ds;
  // sz1    = sz+(Z-z0)/dzds;	          // should be good enough

  // return sz1;
}

//-----------------------------------------------------------------------------
// retrieve event data
void StntupleInitTrackBlock::RetrieveData(AbsEvent* AnEvent) {
  const int verbose(fVerbose);
  mu2e::GeomHandle<mu2e::Tracker> ttHandle;
  tracker = ttHandle.get();

  list_of_algs = 0;
  art::Handle<mu2e::AlgorithmIDCollection> algsHandle;
  AnEvent->getByLabel(fAlgorithmIDCollTag, algsHandle);
  if (algsHandle.isValid()) list_of_algs = (mu2e::AlgorithmIDCollection*) algsHandle.product();

  list_of_kffs = 0;
  list_of_kffs_ptrs = 0;
  int ntrk = 0;
  art::Handle<mu2e::KalSeedCollection> kffcH;
  AnEvent->getByLabel(fKFFCollTag,kffcH);
  if (kffcH.isValid())    {
    list_of_kffs = kffcH.product();
    ntrk = list_of_kffs->size();
    if(verbose > 0) printf("%s::%s: KalSeedCollection %15s has %2i tracks\n",
                           typeid(*this).name(), __func__,
                           fKFFCollTag.encode().c_str(), ntrk);
  } else {
    // attempt to get a kalseed pointer collection by the same name
    art::Handle<mu2e::KalSeedPtrCollection> trkPtrCollH;
    AnEvent->getByLabel(fKFFCollTag,trkPtrCollH);
    if(trkPtrCollH.isValid()) {
      list_of_kffs_ptrs = trkPtrCollH.product();
      ntrk = list_of_kffs_ptrs->size();
      if(verbose > 0) printf("%s::%s: KalSeedPtrCollection %15s has %2i tracks\n",
                             typeid(*this).name(), __func__,
                             fKFFCollTag.encode().c_str(), ntrk);
    } else {
      if(verbose > 0) {
        printf("%s::%s: KalSeedCollection %s not found!\n", typeid(*this).name(), __func__, fKFFCollTag.encode().c_str());
        if(verbose > 1) {
          std::vector<art::Handle<mu2e::KalSeedCollection>> track_collections = AnEvent->getMany<mu2e::KalSeedCollection>();
          printf("  Available KalSeedCollections:\n");
          for(auto coll : track_collections) {
            printf("    %s\n", coll.provenance()->moduleLabel().c_str());
          }
          std::vector<art::Handle<mu2e::KalSeedPtrCollection>> track_ptr_collections = AnEvent->getMany<mu2e::KalSeedPtrCollection>();
          printf("  Available KalSeedPtrCollections:\n");
          for(auto coll : track_ptr_collections) {
            printf("    %s\n", coll.provenance()->moduleLabel().c_str());
          }
        }
      }
    }
  }

  list_of_kff_assns = 0;
  art::Handle<mu2e::KalHelixAssns> kffAssnsH;
  AnEvent->getByLabel(fKFFCollTag,kffAssnsH);
  if (kffAssnsH.isValid()) {
    list_of_kff_assns = kffAssnsH.product();
    const int nassns = list_of_kff_assns->size();
    if(verbose > 0) printf("%s::%s: KalHelixAssns %15s has %2i associations\n", typeid(*this).name(), __func__, fKFFCollTag.encode().c_str(), nassns);
    if(nassns != ntrk) printf("%s::%s: KalHelixAssns %15s has a different number of tracks! %i assns %i trks\n",
                              typeid(*this).name(), __func__, fKFFCollTag.encode().c_str(), nassns, ntrk);
  } else {
    if(verbose > 0) printf("%s::%s: KalHelixAssns %s not found!\n", typeid(*this).name(), __func__, fKFFCollTag.encode().c_str());
  }

  list_of_trk_qual = 0;
  if(fTrkQualCollTag != "") {
    art::Handle<mu2e::MVAResultCollection> trkQualHandle;
    AnEvent->getByLabel(fTrkQualCollTag,trkQualHandle);
    if (trkQualHandle.isValid()) list_of_trk_qual = trkQualHandle.product();
    else if(fTrkQualCollTag != "") printf(" InitTrackBlock::%s: Track quality collection %s not found\n", __func__, fTrkQualCollTag.encode().c_str());
  }

  fSschColl = 0;
  art::Handle<mu2e::ComboHitCollection> sschcH;
  AnEvent->getByLabel(fSsChCollTag,sschcH);
  if (sschcH.isValid()) fSschColl = sschcH.product();
  else if(verbose > 0) printf(" WARNING InitTrackBlock::%s: ComboHitCollection %s not found\n", __func__, fSsChCollTag.encode().c_str());

  list_of_mc_straw_hits = 0;
  art::Handle<mu2e::StrawDigiMCCollection> sdmcHandle;
  AnEvent->getByLabel(fStrawDigiMCCollTag,sdmcHandle);
  if (sdmcHandle.isValid()) list_of_mc_straw_hits = sdmcHandle.product();
  else if(fStrawDigiMCCollTag != "") printf(" InitTrackBlock::%s: StrawDigiMC collection %s not found\n", __func__, fStrawDigiMCCollTag.encode().c_str());

  // list_of_extrapolated_tracks = 0;
  // art::Handle<mu2e::TrkCaloIntersectCollection>  texHandle;
  // AnEvent->getByLabel(fTciCollTag,texHandle);
  // if (texHandle.isValid()) list_of_extrapolated_tracks = texHandle.product();

  list_of_pidp = 0;
  art::Handle<mu2e::PIDProductCollection>  pidpHandle;
  AnEvent->getByLabel(fPIDProductCollTag,pidpHandle);
  if (pidpHandle.isValid()) list_of_pidp = pidpHandle.product();
}

//-----------------------------------------------------------------------------
// set expected hits information
// FIXME: not currently correct due to missing track position vs. z
void StntupleInitTrackBlock::SetExpectedHits(TStnTrack* track,
                                            const mu2e::KalSeed* ks,
                                            const mu2e::Tracker* tracker) {
//-----------------------------------------------------------------------------
// given track parameters, build the expected hit mask
//-----------------------------------------------------------------------------
  double z, zw, dz, dz_min;
  int    iplane, offset;
  int    nz(88);
  const auto& hots = ks->hits();

  for (int iz=0; iz<nz; iz++) {
    z = zmap.fZ[iz];
    // find the track hit closest to that Z
    dz_min = 1.e10;
    for (const auto& hit : hots) {
      const auto sid = hit.strawId();
      const auto& straw = tracker->straw(sid);
      zw = straw.getMidPoint().z();
      dz = z-zw;
      if (std::fabs(dz) < dz_min) {
        dz_min = fabs(dz);
        // closest_hit = hit;
        // closest_z   = zw;
      }
    }
//-----------------------------------------------------------------------------
// found closest hit and the extrapolation length, then extrapolate track
//-----------------------------------------------------------------------------
    // s0  = closest_hit->trkLen();
    //      s   = (z-track->fZ0)/(closest_z-track->fZ0)*s0;

    HepPoint pz(0.,0.,0.); // FIXME      = ks->position(s);

    get_station(tracker,&zmap,z,&iplane,&offset);

    const mu2e::Panel*  panel0 = NULL;
    const mu2e::Panel*  panel;
    const mu2e::Plane*  plane;
    double              min_dphi(1.e10);
    double              dphi, nx, ny, wx, wy, wrho, rho, phi0;
    CLHEP::Hep3Vector   w0mid;
    int                 ipanel;

    plane = &tracker->getPlane(iplane);

    for (int i=0; i<3; i++) {
      ipanel = 2*i+offset;		// panel number
      // check if point pz(x0,y0) overlaps with the segment iseg
      // expected mask is set to zero
      panel = &plane->getPanel(ipanel);
      w0mid = panel->straw0MidPoint();
      // also calculate pho wrt the sector
      wx    = w0mid.x();
      wy    = w0mid.y();

      phi0  = w0mid.phi();            // make sure I understand the range

      wrho  = sqrt(wx*wx+wy*wy);

      nx    = wx/wrho;
      ny    = wy/wrho;

      rho = nx*pz.x()+ny*pz.y();

      dphi = TVector2::Phi_mpi_pi(phi0-pz.phi());
      if ((abs(dphi) < min_dphi) && (rho > wrho)) {
        min_dphi = fabs(dphi);
        panel0   = panel;
      }
    }
//-----------------------------------------------------------------------------
// OK, closest segment found, set the expected bit..
//-----------------------------------------------------------------------------
    if (panel0 != NULL) {
      track->fExpectedHitMask.SetBit(iz,1);
    }
  }
}

//-----------------------------------------------------------------------------
// set hit information
void StntupleInitTrackBlock::SetHitInfo(TStnTrack* track,
                    const mu2e::KalSeed* ks,
                    const std::vector<mu2e::TrkStrawHitSeed>* hots,
                    const mu2e::Tracker* tracker) {

  const int verbose(fVerbose); //control output level for debugging
  for (int j=0; j<50; j++) { // hits by station
    track->fNHPerStation[j] = 0;
  }

  SetExpectedHits(track, ks, tracker);

  // loop through all hits on the track
  int ntrkhits(0), nhitsambig0(0), nwrong(0);
  const int n_kffs_hits = hots->size();
  for (int it=0; it<n_kffs_hits; it++) {
    if(verbose > 5) printf(" Checking hit %i\n", it);
    const auto hit = &hots->at(it);
    if (!hit) continue;
    mu2e::StrawId sid = hit->strawId();
    const mu2e::Straw* straw = &tracker->straw(sid);
    ++ntrkhits;
    const bool is_active = hit->flag().hasAllProperties(mu2e::StrawHitFlag::active);
    if(verbose > 2)
      printf("%3i: active = %o; ID = %5i, plane = %3i, panel = %5i, layer = %5i\n", it, is_active,
        (int) sid.asUint16(), (int) sid.getPlane(),
        (int) sid.getPanelId().asUint16(),
        (int) sid.getLayerId().asUint16());
  //-----------------------------------------------------------------------------
  // the rest makes sense only for active hits
  //-----------------------------------------------------------------------------
    if (is_active) {
      const int loc = hit->index();
      if (hit->ambig() == 0) nhitsambig0 += 1;
      if (loc >= 0 && (list_of_mc_straw_hits) && int(list_of_mc_straw_hits->size()) > loc) {
        if(verbose > 5) printf(" --> MC digi found\n");
        const mu2e::StrawDigiMC* mcdigi = &list_of_mc_straw_hits->at(loc);

        const auto stgs = mcdigi->earlyStrawGasStep().get();
      //-----------------------------------------------------------------------------
      // count number of active hits with R > 200 um and misassigned drift signs
      //-----------------------------------------------------------------------------
        if (stgs) {
          if(verbose > 5) printf(" --> MC gas step found\n");
        //   if (hit->driftRadius() > 0.2) {
        //     const CLHEP::Hep3Vector* v1 = &straw->getMidPoint();
        //     HepPoint p1(v1->x(),v1->y(),v1->z());

        //     CLHEP::Hep3Vector v2 = stgs->position();
        //     HepPoint    p2(v2.x(),v2.y(),v2.z());

        //     TrkLineTraj trstraw(p1,straw->getDirection()  ,0.,0.);

        //     TrkLineTraj trstep (p2,stgs->momvec().unit(),0.,0.);

        //     TrkPoca poca(trstep, 0., trstraw, 0.);

        //     const float mcdoca = poca.doca();
        // //-----------------------------------------------------------------------------
        // // if mcdoca and hit->_iamb have different signs, the hit drift direction has wrong sign
        // //-----------------------------------------------------------------------------
        //     if (hit->ambig()*mcdoca < 0) nwrong += 1;
        //   }
        }
      }

      const mu2e::StrawId& straw_id = straw->id();

      int ist = straw_id.getStation();

      track->fNHPerStation[ist] += 1;

      int pan = straw_id.getPanel();
      int lay = straw_id.getLayer();
      int bit = zmap.fMap[ist][pan][lay];

      track->fHitMask.SetBit(bit,1);
    } // end of active hit check
  } // end of loop over hits on the track
  track->fNHits     = ntrkhits; // ntrkhits | (_kalDiag->_trkinfo._nbend << 16);
  track->fNActive   = ks->nHits() | (nwrong << 16);

  // check the doublet info
  int nd_os(0), nd_ss(0); // number of doublets with opposite sign ambig and same sign ambig (note: not just active doublets)
  int nad(0); // number of active doublets
  for (int it = 0; it < n_kffs_hits - 1; it++) {
    
    const mu2e::TrkStrawHitSeed* ihit = &hots->at(it);
    const mu2e::TrkStrawHitSeed* jhit = &hots->at(it + 1);
    
    if (ihit->strawId().uniquePanel() == jhit->strawId().uniquePanel()) {
      
      if (ihit->flag().hasAllProperties(mu2e::StrawHitFlag::active)
          && jhit->flag().hasAllProperties(mu2e::StrawHitFlag::active)) {
        ++nad;
      }
      
      if (ihit->ambig() == jhit->ambig()) {
        ++nd_ss;
      } else {
        ++nd_os;
      }
    }
  }
  track->fNDoublets = nd_os | (nd_ss << 8) | (nhitsambig0 << 16) | (nad << 24);

  // check the material crossing info
  int nmat(0), nmatactive (0); double radlen(0.);
  if(verbose > 2) printf("Printing material crossing collection:\n");
  for (const auto& straw : ks->straws()) {
    ++nmat;
    if (straw.active()) {
      ++nmatactive;
      radlen += straw._radlen;
    }
    if(verbose > 2) {
      printf("%3i: active = %o; ID = %5i, plane = %2i, panel = %5i, layer = %5i\n", nmat-1, straw.active(),
             (int) straw._straw.asUint16(), (int) straw._straw.getPlane(),
             (int) straw._straw.getPanelId().asUint16(),
             (int) straw._straw.getLayerId().asUint16());
    }
  }

  track->fNMatSites = nmat | (nmatactive << 16);

//-----------------------------------------------------------------------------
// number of MC hits produced by the mother particle
//-----------------------------------------------------------------------------
  track->fNMcStrawHits = 0;
  if (list_of_mc_straw_hits) {
    const int nss_ch = list_of_mc_straw_hits->size();
    for (int i=0; i<nss_ch; i++) {
      const mu2e::StrawDigiMC* mcdigi = &list_of_mc_straw_hits->at(i);
      const mu2e::StrawGasStep* step = mcdigi->earlyStrawGasStep().get();
      if (step) {
        art::Ptr<mu2e::SimParticle> const& simptr = step->simParticle();
        art::Ptr<mu2e::SimParticle> mother        = simptr;
        // while(mother->hasParent())  mother        = mother->parent();
        const mu2e::SimParticle*    sim           = mother.get();
        int sim_id = sim->id().asInt();
        if (sim_id == track->fPartID) {
          track->fNMcStrawHits += 1;
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
// set MC info for the track, using hits directly
void StntupleInitTrackBlock::SetTrackMCInfo(TStnTrack* track,
                                            const mu2e::KalSeed* ks,
                                            AbsEvent* AnEvent) {
  if (!track || !ks) return;
  track->fPdgCode  = 0;
  track->fPartID   = -1;
  track->fNGoodMcHits = ks->nDOF() << 16;
  track->fPFront = -1.f;
  track->fPStOut = -1.f;
  track->fMcDirection = 0;
  if(!fSschColl || !list_of_mc_straw_hits) return; // no hit collections

  // initialize hit information by sim particle 
  const static int max_npart(100);
  int     npart(0), part_nh[max_npart], part_id[max_npart], part_netDir[max_npart];
  int     part_pdg_code[max_npart];
  double  part_first_z[max_npart], part_first_z_p[max_npart];
  double  part_last_z [max_npart];
  
  // particle with the most (active) hits
  int     imax_nh(-1), max_nh(0);

  // loop over track hits
  const auto trk_hits = ks->hits();
  const int n_kffs_hits = trk_hits.size();
  for (int it = 0; it < n_kffs_hits; it++) {
    const auto& hit = trk_hits.at(it);
    const bool is_active = hit.flag().hasAllProperties(mu2e::StrawHitFlag::active);
    if (!is_active) continue;
    const int loc = hit.index();
    if (loc < 0 || int(list_of_mc_straw_hits->size()) <= loc) continue;
    const mu2e::StrawDigiMC& mcdigi = list_of_mc_straw_hits->at(loc);
    const auto stgs = mcdigi.earlyStrawGasStep().get();
    if(!stgs) continue;
    const auto sim = stgs->simParticle().get();
    if(!sim) continue;
    const int id = sim->id().asInt();

    // check if we have seen this sim particle already
    bool found = false;
    int index = 0;
    for (int ip=0; ip<npart; ip++) {
      if (id == part_id[ip]) { //increment N(hits) by this SIM if already seen
        found        = true;
        part_nh[ip] += 1;
        int d = (stgs->momentum().z() >= 0) ? 1 : -1;
        part_netDir[ip] += d;
        const double dz = stgs->position().z();
        if(dz < part_first_z[ip]) {
          part_first_z   [ip] = dz;
          part_first_z_p [ip] = std::sqrt(stgs->momentum().mag2());
        }
        if(dz > part_last_z[ip]) {
          part_last_z    [ip] = dz;
        }
        index = ip;
        break; // exit loop once found
      }
    }

    // if this is the first occurrence, add it to the list
    if (!found) {
      index = npart;
      const double dz = stgs->position().z();
      part_id        [npart] = id;
      part_pdg_code  [npart] = sim->pdgId();
      part_nh        [npart] = 1;
      part_first_z   [npart] = dz;
      part_first_z_p [npart] = std::sqrt(stgs->momentum().mag2());
      part_netDir    [npart] = (stgs->momentum().z() >= 0) ? 1 : -1;
      part_last_z    [npart] = dz;
      npart                 += 1;
    }

    // check if this particle has the most hits so far
    if (part_nh[index] > max_nh) {
      max_nh   = part_nh[index];
      imax_nh  = index;
    }
  }
  if(imax_nh < 0) return; // no sim particle found

  // set the MC information
  track->fPdgCode     = part_pdg_code[imax_nh];
  track->fPartID      = part_id      [imax_nh];
  track->fNGoodMcHits = (ks->nDOF() << 16) + max_nh;
  track->fPFront      = part_first_z_p[imax_nh];
  track->fMcDirection = (part_netDir[imax_nh] >= 0.) ? 1 : -1;

  // check the virtual detector information
  mu2e::GeomHandle<mu2e::VirtualDetector> vdg;
  if(vdg->nDet() == 0) return; // no virtual detectors defined
  art::Handle<mu2e::StepPointMCCollection> vdhits;
  AnEvent->getByLabel(fVdhCollTag,vdhits);
  if(!vdhits.isValid()) return; // no virtual detector hits

  double tfront(1.e10), tstout(1.e10);
  const int nvdhits = vdhits->size();
  for (int i=0; i<nvdhits; i++) {
    const mu2e::StepPointMC& hit = vdhits->at(i);
    mu2e::VirtualDetectorId vdid(hit.volumeId());
    art::Ptr<mu2e::SimParticle> const& simptr = hit.simParticle();
    const mu2e::SimParticle* sim  = simptr.get();
    if(!sim) continue;
    if(int(sim->id().asInt()) != track->fPartID) continue; // not the correct sim particle

    // check if this is front of tracker or stopping target
    if(vdid.isTrackerFront() && hit.time() < tfront) {
      track->fPFront = hit.momentum().mag();
      tfront = hit.time();
    } else if(vdid.isStoppingTarget() && hit.time() < tstout) {
      track->fPStOut = hit.momentum().mag();
      tstout = hit.time();
    }
  }
}


//-----------------------------------------------------------------------------
// set MC info for the track
bool StntupleInitTrackBlock::SetTrackMCInfo(TStnTrack* track, const mu2e::KalSeed* ks,
                                            const mu2e::KalSeedMCAssns& ksmc_assns) {
  if (!track || !ks) return false;
  track->fPdgCode  = 0;
  track->fPartID   = -1;
  track->fNGoodMcHits = ks->nDOF() << 16;
  track->fPFront = -1.f;
  track->fPStOut = -1.f;
  track->fMcDirection = 0;

  // find the KalSeedMC associated with this KalSeed
  const mu2e::KalSeedMC* ksmc(nullptr);
  for (const auto& assn : ksmc_assns) {
    const auto seed = assn.first;
    const auto mc   = assn.second;
    if(!seed || !mc) continue;
    if (&(*ks) == &(*seed)) {
      // found it
      ksmc = &(*mc);
      break;
    }
  }
  if(!ksmc) return false; // no MC info found

  // find the main sim particle for this track
  const mu2e::SimPartStub* main_sim_stub = nullptr;
  for(auto& stub : ksmc->simParticles()) {
    if(!main_sim_stub || stub._nhits > main_sim_stub->_nhits) {
      main_sim_stub = &stub;
    }
  }
  if(!main_sim_stub) return false; // no sim particle found

  track->fPdgCode  = main_sim_stub->_pdg;
  track->fPartID   = main_sim_stub->_spkey.asInt();
  track->fNGoodMcHits = (ks->nDOF() << 16) + main_sim_stub->_nhits;

  // set the track information at the virtual detector steps
  float tfront(1.e10f), tstout(1.e10f);
  for(auto const& vdstep : ksmc->vdSteps()) {
    if(vdstep._vdid.isTrackerFront() && vdstep._time < tfront) {
      track->fPFront = vdstep._mom.R();
      tfront = vdstep._time;
    } else if(vdstep._vdid.isStoppingTarget() && vdstep._time < tstout) {
      track->fPStOut = vdstep._mom.R();
      tstout = vdstep._time;
    }
  }

  // set the track direction using the majority of the hit directions
  int netdir = 0;
  for(auto const& tshmc : ksmc->trkStrawHitMCs()) {
    const auto& hit_sim_stub = ksmc->simParticle(tshmc);
    if(hit_sim_stub._spkey == main_sim_stub->_spkey) {
      const float pz = tshmc.particleMomentum().Z();
      if(pz > 0.f) {
        netdir += 1;
      } else if(pz < 0.f) {
        netdir -= 1;
      }
    }
  }
  track->fMcDirection = (netdir < 0) ? -1 : (netdir > 0) ? 1 : 0;
  return true;
}

//-----------------------------------------------------------------------------
int StntupleInitTrackBlock::InitDataBlock(TStnDataBlock* Block, AbsEvent* AnEvent, Int_t Mode) {
  const char*               oname = {"InitMu2eTrackBlock"};
//-----------------------------------------------------------------------------
// cached pointers, owned by the StntupleMaker_module
//-----------------------------------------------------------------------------
  static int                          initialized(0);

  const int verbose(fVerbose); //control output level for debugging

  int                       ntrk(0), nassns(0), ev_number, sr_number, rn_number;
  TStnTrack*                track;
  TStnTrackBlock            *data(0);

  ev_number = AnEvent->event();
  sr_number = AnEvent->subRun();
  rn_number = AnEvent->run();

  if (Block->Initialized(ev_number,rn_number)) { // FIXME: add subrun?
    if(verbose > 0) printf("%s::%s: Block initialized already, exiting\n", typeid(*this).name(), __func__);
    return 0;
  }


  data = (TStnTrackBlock*) Block;
  data->Clear();

  // get the event data
  RetrieveData(AnEvent);

  if (initialized == 0) {
    initialized = 1;

    InitTrackerZMap(tracker,&zmap);
  }


  ntrk = (list_of_kffs) ? list_of_kffs->size() : (list_of_kffs_ptrs) ? list_of_kffs_ptrs->size() : 0; 
  nassns = (list_of_kff_assns) ? list_of_kff_assns->size() : 0;

  // art::Handle<mu2e::TrackClusterMatchCollection>  tcmH;
  // AnEvent->getByLabel(fTcmCollTag,tcmH);

  art::ServiceHandle<mu2e::GeometryService>   geom;
  mu2e::GeomHandle<mu2e::DetectorSystem>      ds;
  mu2e::GeomHandle<mu2e::VirtualDetector>     vdet;

  const mu2e::AlgorithmID*  alg_id;
  int                       mask;

  const mu2e::Calorimeter* bc(NULL);

  if (geom->hasElement<mu2e::DiskCalorimeter>() ) {
    mu2e::GeomHandle<mu2e::DiskCalorimeter> h;
    bc = (const mu2e::Calorimeter*) h.get();
  }

  for (int itrk=0; itrk<ntrk; itrk++) {
    if(verbose > 1) printf("%s::%s: KalSeedCollection %s track %2i:\n", typeid(*this).name(), __func__, fKFFCollTag.encode().c_str(), itrk);
    track          = data->NewTrack();
    const mu2e::KalSeed* kffs = nullptr;
    if(list_of_kffs) kffs = &list_of_kffs->at(itrk);
    else if(list_of_kffs_ptrs) {
      auto ptr = list_of_kffs_ptrs->at(itrk);
      if(ptr.isAvailable()) kffs = &(*ptr);
      else {
        printf("%s::%s: KalSeedPtrCollection %s track %2i is invalid!\n", typeid(*this).name(), __func__, fKFFCollTag.encode().c_str(), itrk);
        continue;
      }
    }
    if(!kffs) {
      printf("%s::%s: KalSeed %s track %2i not defined!\n", typeid(*this).name(), __func__, fKFFCollTag.encode().c_str(), itrk);
      continue;
    }
    
    // Attempt to find the corresponding helix
    const mu2e::HelixSeed* helix = nullptr;
    if(list_of_kff_assns) {
      for(int iassn = 0; iassn < nassns; ++iassn) {
        const auto assn = list_of_kff_assns->at(iassn);
        const mu2e::KalSeed*   itrack = &(*(assn.first));
        const mu2e::HelixSeed* ihelix = &(*(assn.second));
        if(&(*(itrack)) == &(*kffs)) {
          if(verbose > 1) printf(" --> Associated helix found, association index %i\n", iassn);
          helix = ihelix;
          break;
        }
      }
      if(!helix) printf("%s::%s: KalSeedCollection %s track %2i: Associated helix not found! N(track) = %i N(Assns) = %i\n",
                        typeid(*this).name(), __func__, fKFFCollTag.encode().c_str(), itrk, ntrk, nassns);
    }

//-----------------------------------------------------------------------------
// track-only-based particle ID, initialization ahs already happened in the constructor
//-----------------------------------------------------------------------------
    if (list_of_pidp) {
      const mu2e::PIDProduct* pidp = &list_of_pidp->at(itrk);
      track->fEleLogLHDeDx = pidp->GetLogEProb();
      track->fMuoLogLHDeDx = pidp->GetLogMProb();
      track->fRSlope       = pidp->GetResidualsSlope();
      track->fRSlopeErr    = pidp->GetResidualsSlopeError();
    }

    track->fKalRep[0] = (mu2e::KalSeed*) kffs;
    mask = (0x0001 << 16) | 0x0000;

    if (list_of_algs) {
      alg_id = &list_of_algs->at(itrk);
      mask   = alg_id->BestID() | (alg_id->AlgMask() << 16);
    }

    track->fAlgorithmID = mask;
//-----------------------------------------------------------------------------
// in all cases define momentum at lowest Z - ideally, at the tracker entrance
// 'entlen' - trajectory length, corresponding to the first point in Z (?)
//-----------------------------------------------------------------------------
    // double  h1_fltlen(1.e6), hn_fltlen(1.e6), sent, sexit;
    // const mu2e::TrkStrawHitSeed *first(nullptr), *last(nullptr);

    const std::vector<mu2e::TrkStrawHitSeed>* hots = &kffs->hits();
    int n_kffs_hits = hots->size();
    if(verbose > 1) printf("  N(hits) = %2i:\n", n_kffs_hits);

    // const TrkHit* first = kffs->firstHit()->kalHit()->hit();
    // const TrkHit* last  = kffs->lastHit ()->kalHit()->hit();

    // for (int ih=0; ih<n_kffs_hits; ++ih) {
    //   const mu2e::TrkStrawHitSeed* hit =  &hots->at(ih);
    //   if ((hit != nullptr) and (hit->flag().hasAnyProperty(mu2e::StrawHitFlagDetail::active))) {
    //     if (first == nullptr) first = hit;
    //     last = hit;
    //   }
    // }

    // if (first) h1_fltlen = first->trkLen();
    // if (last ) hn_fltlen = last->trkLen();

    // sent        = std::min(h1_fltlen,hn_fltlen);
    // sexit       = std::max(h1_fltlen,hn_fltlen);
//-----------------------------------------------------------------------------
// find segments corresponding to entry and exit points in the tracker
//-----------------------------------------------------------------------------
    // Intersections with the tracker surfaces
    const mu2e::KalIntersection *kinter_front(nullptr), *kinter_mid(nullptr), *kinter_back(nullptr);
    // Intersections with the stopping target surfaces
    const mu2e::KalIntersection *kinter_st_front(nullptr), *kinter_st_back(nullptr), *kinter_st_outer(nullptr), *kinter_st_inner(nullptr);
    std::vector<const mu2e::KalIntersection *> kinter_st_foils;

    // count relevant intersections along the track extrapolation
    int nst_inters(0), nipa_inters(0), nopa_inters(0), ntsda_inters(0);
    // Take the last intersection (in time) for each surface
    for(size_t ikinter = 0; ikinter < kffs->intersections().size(); ++ikinter) {
      auto const& kinter = kffs->intersections()[ikinter];
      if (kinter.surfaceId() == mu2e::SurfaceIdDetail::ST_Front) {
        if(!kinter_st_front || kinter_st_front->time() < kinter.time()) kinter_st_front = &kinter;
      }
      if (kinter.surfaceId() == mu2e::SurfaceIdDetail::ST_Back) {
        if(!kinter_st_back || kinter_st_back->time() < kinter.time()) kinter_st_back = &kinter;
      }
      if (kinter.surfaceId() == mu2e::SurfaceIdDetail::ST_Outer) {
        if(!kinter_st_outer || kinter_st_outer->time() < kinter.time()) kinter_st_outer = &kinter;
      }
      if (kinter.surfaceId() == mu2e::SurfaceIdDetail::ST_Inner) {
        if(!kinter_st_inner || kinter_st_inner->time() < kinter.time()) kinter_st_inner = &kinter;
      }
      if (kinter.surfaceId() == mu2e::SurfaceIdDetail::ST_Foils) { // save all the foils, ask about them later
        ++nst_inters;
        kinter_st_foils.push_back(&kinter);
      }
      if (kinter.surfaceId() == mu2e::SurfaceIdDetail::IPA) { // count IPA intersections
        ++nipa_inters;
      }
      if (kinter.surfaceId() == mu2e::SurfaceIdDetail::OPA) { // count OPA intersections
        ++nopa_inters;
      }
      if (kinter.surfaceId() == mu2e::SurfaceIdDetail::TSDA) { // count TSdA intersections
        ++ntsda_inters;
      }
      if (kinter.surfaceId() == mu2e::SurfaceIdDetail::TT_Front) {
        if(!kinter_front || kinter_front->time() < kinter.time()) kinter_front = &kinter;
      }
      if (kinter.surfaceId() == mu2e::SurfaceIdDetail::TT_Mid) {
        if(!kinter_mid || kinter_mid->time() < kinter.time()) kinter_mid = &kinter;
      }
      if (kinter.surfaceId() == mu2e::SurfaceIdDetail::TT_Back) {
        if(!kinter_back || kinter_back->time() < kinter.time()) kinter_back = &kinter;
      }

      if(verbose > 2) printf("  Surface %10s: p = %4.1f pz = %5.1f t = %6.1f:\n", kinter.surfaceId().name().c_str(), kinter.mom(), kinter.momentum3().z(), kinter.time());
    }

    // Store the momentum at each surface if found
    if     (kinter_st_front) { track->fPSTFront         = kinter_st_front->mom(); }
    if     (kinter_st_back ) { track->fPSTBack          = kinter_st_back ->mom(); }
    else if(kinter_st_outer) { track->fPSTBack          = kinter_st_outer->mom(); } // if no ST exit sampling, check for an exit through the edge
    else if(kinter_st_inner) { track->fPSTBack          = kinter_st_inner->mom(); }
    if     (kinter_front   ) { track->fPTrackerEntrance = kinter_front   ->mom(); }
    if     (kinter_mid     ) { track->fPTrackerMiddle   = kinter_mid     ->mom(); }
    if     (kinter_back    ) { track->fPTrackerExit     = kinter_back    ->mom(); }
    track->fInterCounts = nst_inters | (nipa_inters << 8) | (nopa_inters << 16) | (ntsda_inters << 24);

    // Decide which intersection to use for the defaults, using the front if available (only Mid available for Online tracks)
    const mu2e::KalIntersection* kinter((kinter_front) ? kinter_front : (kinter_mid) ? kinter_mid : nullptr);
    const mu2e::KalSegment* kseg(nullptr); // only in cases where the intersection isn't found
    if(!kinter) {
      printf("InitTrackBlock::%s: %i:%i:%i: KalSeedCollection %s track %2i: No tracker front/middle intersection!\n",
             __func__, rn_number, sr_number, ev_number, fKFFCollTag.encode().c_str(), itrk);
      double t0;
      auto seg = kffs->t0Segment(t0);
      if(seg != kffs->segments().end()) kseg = &(*seg);
      else printf("InitTrackBlock::%s: %i:%i:%i: KalSeedCollection %s track %2i: --> No t0 KalSegment either!\n",
                  __func__, rn_number, sr_number, ev_number, fKFFCollTag.encode().c_str(), itrk);
    }
    KinKal::VEC3 fitmom = (kinter) ? kinter->momentum3() : (kseg) ? kseg->momentum3() : KinKal::VEC3();
    KinKal::VEC3 pos    = (kinter) ? kinter->position3() : (kseg) ? kseg->position3() : KinKal::VEC3();

    track->fX1 = pos.x();
    track->fY1 = pos.y();
    track->fZ1 = pos.z();

    double px, py, pz;
    px = fitmom.x();
    py = fitmom.y();
    pz = fitmom.z();
//-----------------------------------------------------------------------------
// track parameters in the first point
//-----------------------------------------------------------------------------
    track->Momentum()->SetXYZM(px,py,pz,0.511);
    track->fP         = (kinter) ? kinter->mom()    : (kseg) ? kseg->mom()    : 0.f;
    track->fFitMomErr = (kinter) ? kinter->momerr() : (kseg) ? kseg->momerr() : 0.f;
    track->fPt        = track->Momentum()->Pt();
    track->fChi2      = kffs->chisquared();
    track->fFitCons   = kffs->fitConsistency();
    if(kinter_mid) {
      track->fT0 = kinter_mid->time();
      track->fT0Err = std::sqrt(kinter_mid->loopHelix().paramVar(KinKal::LoopHelix::t0_));
      // track->fFitMomErr = kinter_mid->momerr();
    } else if(kinter) { // use the default intersection if the middle isn't available
      track->fT0 = kinter->time();
      track->fT0Err = std::sqrt(kinter->loopHelix().paramVar(KinKal::LoopHelix::t0_));
    } else if(kseg) { // use the KalSegment if no intersection was found
      track->fT0 = kseg->t0Val();
      track->fT0Err = std::sqrt(kseg->t0Var(mu2e::TrkFitFlag(mu2e::TrkFitFlag::KKLoopHelix)));
    } else {
      track->fT0    = -1.e6;
      track->fT0Err = -1.e6;
      if(kinter) printf("%s::%s: KalSeedCollection %s track %2i: No tracker middle intersection found! Unable to define the time at the tracker center\n",
                        typeid(*this).name(), __func__, fKFFCollTag.encode().c_str(), itrk);
    }

    if(kinter_front) {
      track->fTFront = kinter_front->time();
    }
    if(kinter_back) {
      track->fTBack = kinter_back->time();
    }
//-----------------------------------------------------------------------------
// determine, approximately, 'sz0' - flight length corresponding to the
// virtual detector at the tracker front
//-----------------------------------------------------------------------------
    // Hep3Vector tfront = ds->toDetector(vdet->getGlobal(mu2e::VirtualDetectorId::TT_FrontPA));
    // double     zfront = tfront.z();
    // double     sz0    = s_at_given_z(kffs,zfront);
//-----------------------------------------------------------------------------
// fP0 : track momentum value at Z(TT_FrontPA) - the same as in the first point
// fP2 : track momentum at Z(TT_Back), just for fun, should not be used for anything
//-----------------------------------------------------------------------------
    track->fP0        = track->fP; // can reuse , if needed
    track->fP2        = (kinter_back) ? kinter_back->mom() : 0.f;
//-----------------------------------------------------------------------------
// helical parameters at Z(TT_FrontPA)
//-----------------------------------------------------------------------------
    if(kinter) {
      try {
        KinKal::CentralHelix helx = kinter->centralHelix();
        track->fC0        = helx.omega(); // old
        track->fD0        = helx.d0();
        track->fZ0        = helx.z0();
        track->fPhi0      = helx.phi0();
        track->fTanDip    = helx.tanDip(); // old
        track->fCharge    = helx.charge();
      }
      catch (...) {
        mf::LogWarning(oname) << " ERROR line " << __LINE__ << ": KinKal::CentralHelix trouble" ;
        continue;
      }
    }
    if(verbose > 1) printf("  p = %5.1f, pT = %5.1f, t0 = %6.1f, d0 = %6.1f, z0 = %7.1f, phi0 = %5.2f, tdip = %4.2f, p(ST)-p(Front) = %.2f, N(IPA) = %i\n",
                           track->fP, track->fPt, track->fT0, track->fD0, track->fZ0, track->fPhi0, track->fTanDip,
                           track->fPSTBack - track->fP, track->NIPAIntersections()
                           );

//-----------------------------------------------------------------------------
// virtual detector at the tracker exit: Time at Z(TT_Back)
//-----------------------------------------------------------------------------
//    Hep3Vector vd_tt_back = ds->toDetector(vdet->getGlobal(mu2e::VirtualDetectorId::TT_Back));
    // double     zback      = vd_tt_back.z();
    // double     szb        = s_at_given_z(kffs,zback);

//-----------------------------------------------------------------------------
// initialize hit-related quantities
//-----------------------------------------------------------------------------
    SetHitInfo(track, kffs, hots, tracker);
    if (list_of_trk_qual) track->fTrkQual = list_of_trk_qual->at(itrk)._value;
    else                  track->fTrkQual = -1.e6;
//-----------------------------------------------------------------------------
// print hit info if requested
//-----------------------------------------------------------------------------
    if(verbose > 1) printf("  N(hits) = %2i, N(active) = %2i, N(wrong) = %2i, N(mat) = %3i, N(active mat) = %3i, trkqual = %5.2f\n",
                           track->fNHits, track->NActive(), track->NWrong(),
                           track->NMat(), track->NMatActive(),
                           track->fTrkQual);

//-----------------------------------------------------------------------------
// Either use the KalSeedMCAssns or the available hit information for truth matching
//-----------------------------------------------------------------------------
    art::Handle<mu2e::KalSeedMCAssns> ksmcH;
    AnEvent->getByLabel("SelectReco",ksmcH); // FIXME: hardcoded module label
    // attempt to get the MC information from reco-MC associations
    // if that fails, use hit-based information
    if(!ksmcH.isValid() || !SetTrackMCInfo(track, kffs, *ksmcH)) {
      SetTrackMCInfo(track, kffs, AnEvent); // uses hit-based info
    }
    if(verbose > 1) printf(" Track %i MC info: ID = %3i, PDG = %5i, N(hits) = %2i, P(front) = %5.1f, MC P(ST Out) = %5.1f, MC trajectory = %2i\n",
                           itrk, track->fPartID, track->fPdgCode, track->NGoodMcHits(),
                           track->fPFront, track->fPStOut, track->fMcDirection);

//-----------------------------------------------------------------------------
// consider half-ready case when can't use the extrapolator yet; turn it off softly
//-----------------------------------------------------------------------------
//    const mu2e::TrkCaloIntersect*  extrk;
    CLHEP::Hep3Vector                     x1, x2;
    HepPoint                       extr_point;
    CLHEP::Hep3Vector                     extr_mom;
    // int                            iv, next;
    // TStnTrack::InterData_t*        vint;

    // if (list_of_extrapolated_tracks != NULL) next = list_of_extrapolated_tracks->size();
    // else                                     next = 0;

//     for (int iext=0; iext<next; iext++) {
//       extrk = &list_of_extrapolated_tracks->at(iext);
//       kffs  = extrk->trk().get();
//       if (kffs == track->fKalRep[0]) {
// 	if (track->fExtrk == 0) {
// 	  track->fExtrk = (const mu2e::TrkToCaloExtrapol*) extrk;
// 	}
// 	if (bc) {
// //-----------------------------------------------------------------------------
// // store coordinates of the best intersection in a plane
// //-----------------------------------------------------------------------------
// 	  iv   = extrk->diskId();
// 	  vint = &(track->fDisk[iv]);

// 	  if (vint->fID == -1) {
// 	    vint->fID        = iv;
// 	    vint->fExtrk     = (const mu2e::TrkToCaloExtrapol*) extrk;
// 	    vint->fChi2Match = 1.e6;
// 	  }
// 	  else {
// 	    printf("Run:Event: %i:%i %s : ADDITIONAL EXTR POINT for track %i on vane = %i\n",
// 		   rn_number,ev_number,oname,itrk,iv);
// 	  }
// 	}
//       }
//    }
//-----------------------------------------------------------------------------
// now loop over track-cluster matches and find the right ones to associate
// with the track
//-----------------------------------------------------------------------------
//    unsigned int nm (0);

    // const mu2e::TrackClusterMatchCollection* tcmcoll;

    // if (tcmH.isValid()) {
    //   tcmcoll = tcmH.product();
    //   nm      = tcmcoll->size();
    // }

    // const mu2e::TrackClusterMatch* tcm;

    // double best_chi2_match(1.e6);

//    for (size_t im=0; im<nm; im++) {
//      tcm   = &tcmcoll->at(im);
//      extrk = tcm->textrapol();
//      kffs  = extrk->trk().get();
//      if (kffs == track->fKalRep[0]) {
//	const mu2e::CaloCluster* cl = tcm->caloCluster();
//	iv   = cl->diskID();
//	vint = &track->fDisk[iv];
//	if (bc == 0) {
//	  printf(">>> InitTrackBlock ERROR: %s calorimeter is not defined \n",oname);
//	  continue;
//	}
//
//	x1   = bc->geomUtil().mu2eToDisk(iv,cl->cog3Vector());
//
//	if ((track->fClosestCaloCluster == NULL) || (tcm->chi2() < best_chi2_match )) {
////-----------------------------------------------------------------------------
//// if closest cluster has not been defined or the energy of the new one is higher
//// depending on the calorimeter geometry choice either DX or DZ is meaningful
////-----------------------------------------------------------------------------
//	  track->fClosestCaloCluster = cl;
//	  track->fExtrk              = (const mu2e::TrkToCaloExtrapol*) extrk;
//	  best_chi2_match            = tcm->chi2();
//	}
//
//	vint->fXTrk  = tcm->xtrk();
//	vint->fYTrk  = tcm->ytrk();
//	vint->fZTrk  = tcm->ztrk();
//	vint->fTime  = tcm->ttrk();
//
//	vint->fNxTrk = tcm->nx();
//	vint->fNyTrk = tcm->ny();
//	vint->fNzTrk = tcm->nz();
//
//	if (vint->fCluster == 0) {
//	  vint->fCluster      = cl;
//	  vint->fClusterIndex = tcm->icl();
//	  vint->fEnergy       = cl->energyDep();
//	  vint->fXCl          = x1.x();
//	  vint->fYCl          = x1.y();
//	  vint->fZCl          = x1.z();
//	  vint->fDt           = tcm->dt();
//	  vint->fDx           = tcm->dx();
//	  vint->fDy           = tcm->dy();
//	  vint->fDz           = tcm->dz();
//	  vint->fDu           = tcm->du();
//	  vint->fDv           = tcm->dv();
//	  vint->fChi2Match    = tcm->chi2();
//	  vint->fChi2Time     = tcm->chi2_time();
//	  vint->fIntDepth     = tcm->int_depth();
//	  vint->fPath         = tcm->ds();
//	  vint->fDr           = tcm->dr();
//	  vint->fSInt         = tcm->sint();
//	}
//	else {
//	  printf("%s : ADDITIONAL MATCH for track %i on vane = %i\n", oname,itrk,iv);
//	}
//      }
//    }
//-----------------------------------------------------------------------------
// find intersections to use for electron ID,
// in this version both VMinS and VMaxEp are the same
//-----------------------------------------------------------------------------
    double                    min_chi2_match(1.e6);
    TStnTrack::InterData_t*   v;

    track->fVMinS  = 0;
    track->fVMaxEp = 0;

    int ndisks = bc->nDisks();

    for (int iv=0; iv<ndisks; iv++) {
      v = &track->fDisk[iv];
      if (v->fID >= 0) {
	if (v->fCluster) {
	  if (v->fChi2Match < min_chi2_match) {
	    track->fVMaxEp = v;
	    track->fVMinS  = v;
	    min_chi2_match = v->fChi2Match;
	  }
	}
      }
    }
//-----------------------------------------------------------------------------
// define E/P by the first intersection, if it exists, the second in the
// high-occupancy environment may be unreliable
//----------------------------------------------------
    track->fClusterE = -1.f;
    track->fEp       = -1.f;
    track->fDt       = -1.e12;
    track->fDx       = -1.e12;
    track->fDy       = -1.e12;
    track->fDz       = -1.e12;
    track->fDiskID   = -1;

    const mu2e::CaloCluster* calo_cluster = kffs->caloHit().caloCluster().get();
    if (calo_cluster) {
      track->fClusterE = calo_cluster->energyDep();
      track->fEp = track->fClusterE/track->fP2;
      track->fDiskID = calo_cluster->diskID();
    }

    const bool use_calo_hit(true); // use calo-hit info vs. intersections

    if(!use_calo_hit) {
      if (track->fVMinS != 0) {
        if (track->fVMinS->fCluster) {
          track->fClusterE = track->fVMinS->fCluster->energyDep();
          track->fDx       = track->fVMinS->fDx;
          track->fDy       = track->fVMinS->fDy;
          track->fDz       = track->fVMinS->fDz;
          track->fDt       = track->fVMinS->fDt;
        }
        else {
          //-----------------------------------------------------------------------------
          // intersection with minimal S doesn't have a cluster, check MaxP
          //-----------------------------------------------------------------------------
          if (track->fVMaxEp != 0) {
            if (track->fVMaxEp->fCluster) {
              track->fClusterE = track->fVMaxEp->fCluster->energyDep();
              track->fDx       = track->fVMaxEp->fDx;
              track->fDy       = track->fVMaxEp->fDy;
              track->fDz       = track->fVMaxEp->fDz;
              track->fDt       = track->fVMaxEp->fDt;
            }
          }
        }
      }
    }

//--------------------------------------------------------------------------------
// now set the parameters associated with the TrkCaloHit
//--------------------------------------------------------------------------------
    const mu2e::TrkCaloHitSeed*  tch;
    TStnTrack::InterData_t*      vtch;

    tch = &kffs->caloHit();
    // for (auto it : hots) {
    //   tch   = itdynamic_cast<const mu2e::TrkCaloHit*> (*it);
    //-----------------------------------------------------------------------------
    // skip TrkStrawHit hit
    //-----------------------------------------------------------------------------
    if (tch) {
      vtch = &(track->fTrkCaloHit);
      const mu2e::CaloCluster* cl = tch->caloCluster().get();

      if (cl) {
	CLHEP::Hep3Vector cpos = bc->geomUtil().mu2eToTracker(bc->geomUtil().diskFFToMu2e( cl->diskID(), cl->cog3Vector()));

	CLHEP::Hep3Vector pos;
	// tch->hitPosition(pos);

	vtch->fID           = cl->diskID();		//
	vtch->fClusterIndex = -1;         // cluster index in the list of clusters

      // the following includes the (Calibrated) light-propagation time delay.  It should eventually be put in the reconstruction FIXME!
      // This velocity should come from conditions FIXME!

	vtch->fTime         = tch->t0().t0();          // extrapolated track time, not corrected by _dtOffset
	vtch->fEnergy       = cl->energyDep();            // cluster energy
	vtch->fXTrk         = pos.x();
	vtch->fYTrk         = pos.y();
	vtch->fZTrk         = pos.z();
	// vtch->fNxTrk        = -9999.;		// track direction cosines in the intersection point
	// vtch->fNyTrk        = -9999.;
	// vtch->fNzTrk        = -9999.;
	vtch->fXCl          = cpos.x();			// cluster coordinates
	vtch->fYCl          = cpos.y();
	vtch->fZCl          = cpos.z();
	vtch->fDx           = vtch->fXTrk - vtch->fXCl;	// TRK-CL
	vtch->fDy           = vtch->fYTrk - vtch->fYCl;	// TRK-CL
	vtch->fDz           = vtch->fZTrk - vtch->fZCl;	// TRK-CL
	vtch->fDt           = tch->_udt; //tch->t0().t0() - tch->time();
	// vtch->fDu           = -9999.;			// ** added in V6
	// vtch->fDv           = -9999.;			// ** added in V6
	// vtch->fChi2Match    = -9999.;		// track-cluster match chi&^2 (coord)
	// vtch->fChi2Time     = -9999.;		// track-cluster match chi&^2 (time)
	vtch->fPath         = tch->hitLen();			// track path in the disk
	vtch->fIntDepth     = -9999.;                     // ** added in V6 :assumed interaction depth
	vtch->fDr           = tch->_udoca; // clusterAxisDOCA(); // tch->poca().doca();         // distance of closest approach
	// vtch->fSInt         = -9999.;                 // ** added in V10: interaction length, calculated
	vtch->fCluster      = cl;
	//    vtch->fExtrk        = NULL;

        if(use_calo_hit) {
          track->fDx     = tch->_udoca;
          track->fDy     = 0.f; // track->fVMinS->fDy;
          track->fDz     = 0.f; // track->fVMinS->fDz;
          track->fDt     = tch->_udt;
        }
      }
    }
  }
					// on return set event and run numbers
					// to mark block as initialized
  data->f_RunNumber   = rn_number;
  data->f_EventNumber = ev_number;

  return 0;
}

//-----------------------------------------------------------------------------
// 2015-04-02: this routine is not finished yet
//-----------------------------------------------------------------------------
Int_t StntupleInitTrackBlock::ResolveLinks(TStnDataBlock* Block, AbsEvent* AnEvent, int Mode)
{
  int    ev_number, rn_number;

  ev_number = AnEvent->event();
  rn_number = AnEvent->run();

  if (! Block->Initialized(ev_number,rn_number)) return -1;
//-----------------------------------------------------------------------------
// do not do initialize links for 2nd time
//-----------------------------------------------------------------------------
  if (Block->LinksInitialized()) return 0;

  TStnTrackBlock* tb = (TStnTrackBlock*) Block;

  art::Handle<mu2e::CaloClusterCollection> cch;
  if (not fCaloClusterCollTag.empty()) {
    AnEvent->getByLabel(fCaloClusterCollTag,cch);
  }
//-----------------------------------------------------------------------------
// TStnTrackBlock is already initialized
//-----------------------------------------------------------------------------
  TStnEvent* ev = Block->GetEvent();

  if (not fTrackTsCollTag.empty()) {
//-----------------------------------------------------------------------------
// seeds are stored, fill links part: 'TrackTs' collection stores, for each track,
// its KalSeed
//-----------------------------------------------------------------------------
    art::Handle<mu2e::KalSeedCollection>  ksch;
    mu2e::KalSeedCollection*              list_of_kalSeeds;

    AnEvent->getByLabel(fTrackTsCollTag,ksch);
    list_of_kalSeeds = (mu2e::KalSeedCollection*) ksch.product();

    TStnTrackSeedBlock* tsb = (TStnTrackSeedBlock*) ev->GetDataBlock(fTrackTsBlockName.Data());

    int    ntrk = tb->NTracks();
    int    nts  = tsb->NTrackSeeds();

    for (int i=0; i<ntrk; i++) {
      TStnTrack* trk = tb->Track(i);
      const mu2e::KalSeed *ts = &list_of_kalSeeds->at(i); // seed corresponding to track # i
      int  loc(-1);
      for (int j=0; j<nts; ++j) {
	const mu2e::KalSeed* tss = tsb->TrackSeed(j)->fTrackSeed;
	if (ts == tss) {
	  loc = j;
	  break;
	}
      }

      if (loc < 0) {
	printf(">>> ERROR: %s track %i -> no TrackSeed associated\n",
               fKFFCollTag.encode().data(), i);
	continue;
      }

      trk->SetTrackSeedIndex(loc);
    }
  }
//-----------------------------------------------------------------------------
// mark links as initialized
//-----------------------------------------------------------------------------
  Block->SetLinksInitialized();

  return 0;
}
