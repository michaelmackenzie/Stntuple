///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

#include "Stntuple/mod/InitCrvPulseBlock.hh"
#include "Offline/RecoDataProducts/inc/CrvRecoPulse.hh"
// #include "Offline/RecoDataProducts/inc/CrvCoincidence.hh"
#include "Offline/RecoDataProducts/inc/CrvCoincidenceCluster.hh"

//-----------------------------------------------------------------------------
// in this case AbsEvent is just not used
//-----------------------------------------------------------------------------
int StntupleInitCrvPulseBlock::InitDataBlock(TStnDataBlock* Block, AbsEvent* Event, int Mode) {

  int ev, rn, sr;

  ev = Event->event();
  rn = Event->run();
  sr = Event->subRun();

  if (Block->Initialized(ev,rn,sr)) return 0;
  
  TCrvPulseBlock* block = (TCrvPulseBlock*) Block;

  block->f_EventNumber  = ev;
  block->f_RunNumber    = rn;
  block->f_SubrunNumber = sr;
//-----------------------------------------------------------------------------
// store CrvRecoPulse's, don't store pulse waveforms
//-----------------------------------------------------------------------------
  art::Handle<mu2e::CrvRecoPulseCollection> crpch;
  const mu2e::CrvRecoPulseCollection*       crpc(nullptr);
  int   ncrp(0);

  if (! fCrvRecoPulseCollTag.empty() != 0) {
    bool ok = Event->getByLabel(fCrvRecoPulseCollTag,crpch);
    if (ok) { 
      crpc = crpch.product();
      ncrp = crpc->size();
    }
  }
  
  // const mu2e::CrvRecoPulse* p0(nullptr);
  // if (ncrp > 0) p0 = &crpc->at(0);

  for (int i=0; i<ncrp; i++) {
    const mu2e::CrvRecoPulse* ralph = &crpc->at(i);

    TCrvRecoPulse* pulse = block->NewPulse(); // increments block->fNPulses

    int   npes        = ralph->GetPEs();
    int   npes_height = ralph->GetPEsPulseHeight();
    int   nind        = ralph->GetWaveformIndices().size();
    int   bar         = ralph->GetScintillatorBarIndex().asInt();
    int   sipm        = ralph->GetSiPMNumber();

    float time        = ralph->GetPulseTime();
    float height      = ralph->GetPulseHeight();
    float width       = ralph->GetPulseBeta(); // was GetPulseWidth();
    float chi2        = ralph->GetPulseFitChi2();
    float le_time     = ralph->GetLEtime();

    pulse->Set(i,npes,npes_height,nind,bar,sipm,time,height,width,chi2,le_time);
  }
//-----------------------------------------------------------------------------
// store CrvCoincidences's
//-----------------------------------------------------------------------------
  // art::Handle<mu2e::CrvCoincidenceCollection> ccch;
  // const mu2e::CrvCoincidenceCollection*       ccc(nullptr);
  // int                                         ncc(0);

  // if (! fCrvCoincidenceCollTag.empty()) {
  //   bool ok = Event->getByLabel(fCrvCoincidenceCollTag,ccch);
  //   if (ok) { 
  //     ccc = ccch.product();
  //     ncc = ccc->size();
  //   }
  // }
  
  // for (int icc=0; icc<ncc; icc++) {
  //   const mu2e::CrvCoincidence* ralph_cc = &ccc->at(icc);

  //   TCrvCoincidence* cc = block->NewCoincidence();  // increments block->fNCoincidences

  //   const std::vector<art::Ptr<mu2e::CrvRecoPulse>>* list_of_pulses = &ralph_cc->GetCrvRecoPulses();

  //   int sector      = ralph_cc->GetCrvSectorType();
  //   int np          = list_of_pulses->size();

  //   cc->Set(icc,sector,np);

  //   for (int i=0; i<np; i++) {
  //     int index = list_of_pulses->at(i).get()-p0;
  //     block->fCoincidencePulseLinks->Add(icc,index);
  //   }
  // }

  return 0;
}


//-----------------------------------------------------------------------------
// keep this function as an example, don't really need it
//-----------------------------------------------------------------------------
int StntupleInitCrvPulseBlock::ResolveLinks(TStnDataBlock* Block, AbsEvent* Event, int Mode) {

  int ev, rn, sr;
  
  ev = Event->event();
  rn = Event->run();
  sr = Event->subRun();
  
  if (! Block->Initialized(ev,rn,sr)) return -1;

// 					// do not do initialize links 2nd time

//   if (Block->LinksInitialized()) return 0;
// //-----------------------------------------------------------------------------
// // block is initialized, links - not yet
// //-----------------------------------------------------------------------------
//   TCrvPulseBLock* block = (TCrvPulseBlock*) Block;

//   art::Handle<mu2e::CrvRecoPulseCollection> crpch;
//   const mu2e::CrvRecoPulseCollection* crpc(nullptr);
//   int   ncrp(0);

//   if (fCrvRecoPulseCollTag.data()[0] != 0) {
//     art::InputTag tag(fCrvRecoPulseCollTag);
//     bool ok = Event->getByLabel(tag,crph);
//     if (ok) { 
//       crpc = crpch->product();
//       ncrp = crpc->size();
//     }
//   }

// //-----------------------------------------------------------------------------
// // handle Coincidences-to-Pulse links
// //-----------------------------------------------------------------------------
//   art::Handle<mu2e::CrvCoincidenceCollection> ccch;
//   const mu2e::CrvCoincidenceCollection*       ccc(nullptr);
//   int                                         ncc(0);

//   if (fCrvCoincidenceCollTag.data()[0] != 0) {
//     art::InputTag tag(fCrvCoincidenceCollTag);
//     bool ok = Event->getByLabel(tag,ccch);
//     if (ok) { 
//       ccc = ccch->product();
//       ncc = ccc->size();
//     }
//   }
  
//   for (int i=0; i<ncc; i++) {
//     const mu2e::CrvCoincidence* ralph_cc = ccc->at(i).get();

//     TCrvCoincidence* cc = block->NewCoincidence();

//     const std::vector<art::Ptr<mu2e::CrvRecoPulse>>* list_of_pulses = &ralph_cc->GetCrvRecoPulses();

//     int sector      = ralph_cc->GetCrvSectorType();
//     int np          = list_of_pulses->size();

//     cc->Set(i,sector,np);

//     for (int ip=0; i<np) {
//       int index = list_of_pulses->at(i).get()-p0;
//       fCoincidenceToPulseLinks->Add(i,index);
//     }
//   }

  return 0;
}
