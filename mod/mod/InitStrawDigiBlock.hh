///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __InitStrawDigiBlock__
#define __InitStrawDigiBlock__

#include <string.h>

#include "canvas/Utilities/InputTag.h"

#include "Offline/TrackerConditions/inc/TrackerPanelMap.hh"
#include "Offline/ProditionsService/inc/ProditionsHandle.hh"

#include "Stntuple/obj/TStnInitDataBlock.hh"

class StntupleInitStrawDigiBlock : public TStnInitDataBlock {
public:
  
  struct WfParam_t {
    int   fs;                         // first sample above _minPulseHeight
    float bl;                         // baseline
    float ph;                         // pulse height
    float q;                          // Q(positive)
    float qt;                         // Q(tail)
  };

  art::InputTag   fStrawDigiCollTag;

  int             fLastRun;
  int             fNAdcSamples;
  int             fNSamplesBL;
  float           fMinPulseHeight;
  float           fMinSDPHToSave;
  
  mu2e::ProditionsHandle<mu2e::TrackerPanelMap>   fTpm_h;
  const mu2e::TrackerPanelMap*                    fTrkPanelMap;
//-----------------------------------------------------------------------------
// functions
//-----------------------------------------------------------------------------
public:

  StntupleInitStrawDigiBlock();
  
  void        SetStrawDigiCollTag(art::InputTag& Tag) { fStrawDigiCollTag = Tag; }
  //   void   SetStrawDigiMCCollTag (art::InputTag& Tag) { fStrawDigiMCCollTag = Tag; }

  int         ProcessAdcWaveform (float* Wf, WfParam_t* Wp);
  
  virtual int InitDataBlock      (TStnDataBlock* Block, AbsEvent* Evt, int Mode);
  virtual int ResolveLinks       (TStnDataBlock* Block, AbsEvent* Evt, int Mode);

};

#endif
