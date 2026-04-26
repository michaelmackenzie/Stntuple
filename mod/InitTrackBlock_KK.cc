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



// #include "Offline/BTrkData/inc/Doublet.hh"

// #include "Offline/RecoDataProducts/inc/TrkCaloIntersect.hh"
// #include "Offline/RecoDataProducts/inc/TrackClusterMatch.hh"
#include "Offline/RecoDataProducts/inc/HelixSeed.hh"
#include "Offline/RecoDataProducts/inc/KalSeed.hh"

#include "Offline/RecoDataProducts/inc/KalSeedAssns.hh"

#include "Offline/MCDataProducts/inc/GenParticle.hh"
#include "Offline/MCDataProducts/inc/SimParticle.hh"
#include "Offline/MCDataProducts/inc/StrawGasStep.hh"
#include "Offline/DataProducts/inc/VirtualDetectorId.hh"

#include "Offline/RecoDataProducts/inc/StrawDigi.hh"
#include "Offline/RecoDataProducts/inc/StrawHit.hh"
#include "Offline/RecoDataProducts/inc/CaloHit.hh"
#include "Offline/RecoDataProducts/inc/CaloCluster.hh"
#include "Offline/RecoDataProducts/inc/AlgorithmID.hh"

					          // BaBar 
// #include "BTrk/ProbTools/ChisqConsistency.hh"
// #include "BTrk/BbrGeom/BbrVectorErr.hh"
// #include "BTrk/BbrGeom/TrkLineTraj.hh"
// #include "BTrk/TrkBase/TrkPoca.hh"
// #include "BTrk/KalmanTrack/KalHit.hh"

#include "Stntuple/mod/InitTrackBlock_KK.hh"
//-----------------------------------------------------------------------------
// 2023-06-29: links directly to helices
//-----------------------------------------------------------------------------
Int_t StntupleInitTrackBlock_KK::ResolveLinks(TStnDataBlock* Block, AbsEvent* AnEvent, int Mode) 
{
  const char* oname = {"stntuple::InitTrackBlock_KK::ResolveLinks"};
  int    ev_number, rn_number;

  ev_number = AnEvent->event();
  rn_number = AnEvent->run();

  if (! Block->Initialized(ev_number,rn_number)) return -1;
//-----------------------------------------------------------------------------
// do not do initialize links for 2nd time
//-----------------------------------------------------------------------------
  if (Block->LinksInitialized()) return 0;

  art::Handle<mu2e::KalHelixAssns> ksfhaH;
  const mu2e::KalHelixAssns* ksfha;
  AnEvent->getByLabel(fKFFCollTag, ksfhaH);
  if (ksfhaH.isValid()) {ksfha = ksfhaH.product();}
  else {
    // Online trigger paths typically don't have this, so ignore this in normal running
    if(fVerbose > 0) printf(" WARNING in InitTrackBlock_KK::%s: KalHelixAssns %s not found!\n", __func__, fKFFCollTag.encode().c_str());
    ksfha = nullptr;
  }

  TStnTrackBlock* tb = (TStnTrackBlock*) Block;
  TStnEvent*      ev = Block->GetEvent();
  TStnHelixBlock* hb = (TStnHelixBlock*) ev->GetDataBlock(fTrackHsBlockName.Data());
  if(!hb) {
    printf(" WARNING in InitTrackBlock_KK::%s: Helix block %s not found!\n", __func__, fTrackHsBlockName.Data());
  }

  int nt = (tb) ? tb->NTracks() : 0;
  int nh = (hb) ? hb->NHelices() : 0;
  
  for (int i=0; i<nt; i++) {
    TStnTrack* trk = tb->Track(i);
//-----------------------------------------------------------------------------
// this is just legacy - out of 4 elements, only the first one is used
//-----------------------------------------------------------------------------
    const mu2e::KalSeed* ksf = trk->fKalRep[0];
//-----------------------------------------------------------------------------
// looking for the seed in associations
//-----------------------------------------------------------------------------
    const mu2e::HelixSeed* hs(nullptr);
    if (ksfha) {
      for (auto ass: *ksfha) {
        const mu2e::KalSeed* qsf = ass.first.get();
        if (qsf == ksf) {
          if(fVerbose > 1) printf("%s::%s: KalSeedCollection %s track %2i: Associated helix found\n",
                            typeid(*this).name(), __func__, fKFFCollTag.encode().c_str(), i);
          hs = ass.second.get();
          break;
        }
      }
    }

    int  hindex(-1);

    if (hs == nullptr && ksfha) {
      mf::LogWarning(oname) << " WARNING in " << oname << ":" << __LINE__
                            << ": kseed->helix() is gone."
                            << " len(assn) = " << ksfha->size() << " --> FIXIT" ;
    }
    else if(ksfha) {
//-----------------------------------------------------------------------------
// search for the helix in the helix block
//-----------------------------------------------------------------------------
      const mu2e::HelixSeed* hs2(nullptr);
      for (int j=0; j<nh; ++j){
        TStnHelix* hel  = hb->Helix(j);
        hs2 = hel->fHelix;
        if (hs2 == hs){
          hindex = j;
          break;
        }    
      }
    }
    
    if (hindex < 0 && ksfha) {
      mf::LogWarning(oname) << " WARNING in " << oname << ":" << __LINE__ 
                            << ": tracjseed " << fKFFCollTag.encode().data() 
                            << ":" << i << " has no HelixSeed associated" ;
    }
    trk->SetHelixIndex(hindex);
  }
//-----------------------------------------------------------------------------
// mark links as initialized
//-----------------------------------------------------------------------------
  Block->SetLinksInitialized();

  return 0;
}

