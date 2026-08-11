//-----------------------------------------------------------------------------
//  Apr 2016 G. Pezzullo: initialization of the MU2E STNTUPLE TimePeak block
//  2020-10-18 P.M. rewrite 
//-----------------------------------------------------------------------------
#include <cstdio>
#include <format>

#include "TROOT.h"
#include "TFolder.h"
#include "TLorentzVector.h"
#include "TVector2.h"

#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Event.h"

#include "Offline/RecoDataProducts/inc/StrawDigi.hh"

#include "Stntuple/obj/TStrawDigiBlock.hh"
#include "Stntuple/obj/TStnEvent.hh"

#include "Stntuple/mod/InitStrawDigiBlock.hh"
#include "Stntuple/mod/MFInterface.hh"

//-----------------------------------------------------------------------------
StntupleInitStrawDigiBlock::StntupleInitStrawDigiBlock() {

  fTrkPanelMap    = nullptr;

  fLastRun        = -1;
  fNAdcSamples    = -1;
  fNSamplesBL     =  6;
  fMinPulseHeight =  5.;
  fMinSDPHToSave  = -1.;
}

//-----------------------------------------------------------------------------
int StntupleInitStrawDigiBlock::ProcessAdcWaveform(float* Wf, WfParam_t* Wp) {
//-----------------------------------------------------------------------------
// waveform processing
// 1. determine the baseline
//-----------------------------------------------------------------------------
  Wp->bl = 0;
  for (int i=0; i<fNSamplesBL; ++i) {
    Wp->bl += Wf[i];
  }
  Wp->bl = Wp->bl/fNSamplesBL;
//-----------------------------------------------------------------------------
// 2. subtract the baseline and calculate the charge
//-----------------------------------------------------------------------------
  for (int i=0; i<fNAdcSamples; i++) {
    Wf[i] = Wf[i]-Wp->bl;
  }

  int   tail  = 0;
  Wp->fs = -1;
  Wp->q  = 0;
  Wp->qt = 0;
  Wp->ph = -1;
  for (int i=fNSamplesBL; i<fNAdcSamples; ++i) {
    if (Wf[i] > fMinPulseHeight) {
      if (tail == 0) {
                                        // first sample above the threshold
        if (Wp->fs < 0) Wp->fs = i;

        Wp->q += Wf[i];
        if (Wf[i] > Wp->ph) {
          Wp->ph = Wf[i];
        }
      }
    }
    else if (Wf[i] < 0) {
      if (Wp->ph > 0) {
        tail  = 1;
      }
      if (tail == 1) Wp->qt -= Wf[i];
    }
  }
//-----------------------------------------------------------------------------
// done
//-----------------------------------------------------------------------------
  // if (Wp->q < 100) {
  //   TLOG(TLVL_DEBUG+1) << "event=" << _art_event->run() << ":"
  //                      << _art_event->subRun() << ":" << _art_event->event() 
  //                      << " Q=" << Wp->q;
  // }
  return 0;
}

//-----------------------------------------------------------------------------
// assume that the collection name is set, so we could grab it from the event
// ComboHitCollection and StrawHitCollection are of the same time, the first one 
// contains real combo hits (one combo hit could be made out of more than one straw hit),
// the other one has a combo hit per straw digi
//-----------------------------------------------------------------------------
int  StntupleInitStrawDigiBlock::InitDataBlock(TStnDataBlock* Block, AbsEvent* Evt, int Mode) {
  // const char* oname = {"StntupleInitStrawDigiBlock::InitDataBlock"};

  TStrawDigiBlock* block = (TStrawDigiBlock*) Block;
  
  block->Clear();
  
  const mu2e::StrawDigiCollection*        sdc(nullptr);
  
  art::Handle<mu2e::StrawDigiCollection>  sdch;
  int                                     ndigis(0);
  
  art::Handle<mu2e::StrawDigiADCWaveformCollection> sdawfch;
  const mu2e::StrawDigiADCWaveformCollection*       sdawfc(nullptr);
  
  art::EventID evt_id = Evt->id();
  
  if (fLastRun != (int) Evt->run()) {
    
    fTrkPanelMap = &fTpm_h.get(evt_id);
    fLastRun    = Evt->run();
  }

  if (! fStrawDigiCollTag.empty()) {
    bool ok = Evt->getByLabel(fStrawDigiCollTag,sdch);
    if (ok) {
      sdc   = sdch.product();
      ndigis = sdc->size();
    }
    else {
      // no cal digi collection: print diagnostics but do nothing else, just leave the data block empty
      stntuple::print_(&evt_id,
                       stntuple::e_WARNING,std::format("ERROR: no StrawDigiCollection tag={} found. BAIL OUT",
                                                       fStrawDigiCollTag.encode().data()));
      return 0;
    }
    
    ok =  Evt->getByLabel(fStrawDigiCollTag,sdawfch);
    if (ok) { 
      sdawfc = sdawfch.product();
    }
    else {
      stntuple::print_(&evt_id,
                       stntuple::e_WARNING,
                       std::format("WARNING: StrawDigiADCWaveformCollection:{:s} is not available. Bail out\n",
                                   fStrawDigiCollTag.encode().data()));
      return 0;
    }
  }
  
  int idigi=0;
  for (int i=0; i<ndigis; i++) {
    const mu2e::StrawDigi*            sd    = &sdc->at(i);
    const mu2e::StrawDigiADCWaveform* sdawf = &sdawfc->at(i);
    int ns = sdawf->samples().size();
    // expect the same number of samples for all straw digi waveforms
    if (fNAdcSamples == -1) fNAdcSamples = ns;
//-----------------------------------------------------------------------------
// process the waveform and store the waveform parameters
//-----------------------------------------------------------------------------
    float wf[100];
    for (int is=0; is<ns; is++) {
      wf[is] = sdawf->samples()[is];
    }

    WfParam_t wp;
    ProcessAdcWaveform(wf,&wp);
//-----------------------------------------------------------------------------
// for calibrations, write out a special small size ntuple with the cut on the SD pulse height
// by default, _minSdPulseHeight is -1.
//-----------------------------------------------------------------------------
    if (wp.ph < fMinSDPHToSave)                             continue;
    TStrawDigi* tsd           = block->NewDigi();
    idigi++;

    tsd->Init(fNAdcSamples);
    
    tsd->fSid          = sd->strawId().asUint16();
 
    int pln = sd->strawId().plane();
    int pnl = sd->strawId().panel();
    // int ich = sd->strawId().straw();

    const mu2e::TrkPanelMap::Row* tpm = fTrkPanelMap->panel_map_by_offline_ind(pln,pnl);

    int dtc_id    = tpm->dtc();

    tsd->fMnid         = tpm->mnid();
    
    tsd->fTdc0         = sd->TDC(mu2e::StrawEnd::cal);
    tsd->fTdc1         = sd->TDC(mu2e::StrawEnd::hv );
    tsd->fTot0         = sd->TOT(mu2e::StrawEnd::cal);
    tsd->fTot1         = sd->TOT(mu2e::StrawEnd::hv );
    
    if (fNAdcSamples != (int) tsd->fAdc.size()) {
      tsd->fAdc.resize(fNAdcSamples);
    }
    for (int i=0; i<fNAdcSamples; i++) {
      tsd->fAdc[i] = sdawf->samples()[i];
    }

    // straw digi PMP is not a pulse-minus-pedestal, but the event window tag % 1024
    // stored there for debugging purposes
    // those shoudl be the same for all DTCs
    tsd->fPmp          = sd->PMP();
    // tracker dtc_id runs from 1 to 36
    if (block->fEwTag1024[dtc_id-1] == -1) {
      block->fEwTag1024[dtc_id-1] = tsd->fPmp;
    }
    else if (block->fEwTag1024[dtc_id-1] !=  tsd->fPmp) {
      stntuple::print_(&evt_id,
                       stntuple::e_ERROR,
                       std::format("dtc_id:{} block->pmp[dtc_id]:{} hit_pmp{}",dtc_id,
                                   block->fEwTag1024[dtc_id-1],tsd->fPmp));
    }
    
    tsd->fFs = wp.fs;
    tsd->fBl = wp.bl;
    tsd->fPh = wp.ph;
  }


  block->fNDigis = idigi;

  return 0;
}

//_____________________________________________________________________________
Int_t StntupleInitStrawDigiBlock::ResolveLinks(TStnDataBlock* Block, AbsEvent* AnEvent, int Mode) {
  // Mu2e version, do nothing

//   Int_t  ev_number, rn_number;

//   ev_number = AnEvent->event();
//   rn_number = AnEvent->run();

//   if (! Block->Initialized(ev_number,rn_number)) return -1;

// 					// do not do initialize links 2nd time

//   if (Block->LinksInitialized()) return 0;

//   TStnEvent*                 ev;
//   TStnTimeClusterBlock*      hb;

//   TStnHelixBlock*            tsb;
//   TStnHelix*                 helixseed;

//   const mu2e::TimeCluster*   ktcluster, *fktcluster;
//   const mu2e::HelixSeed*     kseed;

//   char                       short_tcluster_block_name[100];

//   ev     = Block->GetEvent();
//   hb     = (TStnTimeClusterBlock*) Block;
  
//   hb->GetModuleLabel("mu2e::TimeClusterCollection"  , short_tcluster_block_name);

//   tsb    = (TStnHelixBlock*) ev->GetDataBlock(short_tcluster_block_name);
  
//   int    ntc(0);
//   if (hb!=nullptr){
//     ntc = hb ->NTimeClusters();
//   }
//   int    nhelixseed(0);
//   if (tsb !=nullptr){
//     nhelixseed = tsb->NHelices();
//   }

//   for (int i=0; i<ntc; ++i){
//     TStnTimeCluster* tc = hb->TimeCluster(i);
//     ktcluster = tc->fTimeCluster;
//     int      helixseedIndex(-1);
//     for (int j=0; j<nhelixseed; ++j){
//       helixseed   = tsb->Helix(j);
//       kseed       = helixseed->fHelix;
//       fktcluster  = kseed->timeCluster().get();
//       if (fktcluster == ktcluster) {
// 	helixseedIndex = j;
// 	break;
//       }
//     }
    
//     if (helixseedIndex < 0) {
//       printf(">>> ERROR: TimeClusterFinder timeCluster %i -> no HelixSeed associated\n", i);//FIXME!
// 	  continue;
//     }
    
//     tc->SetHelixSeedIndex(helixseedIndex);
//   }
// //-----------------------------------------------------------------------------
// // mark links as initialized
// //-----------------------------------------------------------------------------
//   hb->fLinksInitialized = 1;

  return 0;
}


