///////////////////////////////////////////////////////////////////////////////
// interactive 2D event display. 
//
// $Id: MuHitDisplay_module.cc,v 1.6 2014/09/20 17:54:06 murat Exp $
// $Author: murat $
// $Date: 2014/09/20 17:54:06 $
//
// Contact person:  Pavel Murat, Gianantonio Pezzulo
///////////////////////////////////////////////////////////////////////////////
#include <boost/algorithm/string.hpp>

#include "TApplication.h"

#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art_root_io/TFileService.h"

#include "Offline/GlobalConstantsService/inc/GlobalConstantsHandle.hh"
#include "Offline/GlobalConstantsService/inc/PhysicsParams.hh"
#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"
#include "Offline/ProditionsService/inc/ProditionsHandle.hh"

#include "Stntuple/gui/TEvdTracker.hh"
#include "Stntuple/gui/TEvdStation.hh"
#include "Stntuple/gui/TEvdPlane.hh"
#include "Stntuple/gui/TEvdPanel.hh"

#include "Stntuple/gui/THeaderVisNode.hh"
#include "Stntuple/gui/TCalVisNode.hh"
#include "Stntuple/gui/TCrvVisNode.hh"
#include "Stntuple/gui/TTrkVisNode.hh"
#include "Stntuple/gui/TEvdHelixVisNode.hh"
#include "Stntuple/gui/TEvdTimeClusterVisNode.hh"
#include "Stntuple/gui/TEvdPanel.hh"
#include "Stntuple/gui/TEvdPanelVisNode.hh"

#include "Stntuple/gui/TMcTruthVisNode.hh"
#include "Stntuple/gui/TCalView.hh"
#include "Stntuple/gui/TCrvView.hh"

#include "Stntuple/print/TAnaDump.hh"

#include "Stntuple/mod/MuHitDisplay_module.hh"

#include "Stntuple/obj/TSimpBlock.hh"
#include "Stntuple/mod/InitSimpBlock.hh"

using namespace std;

namespace mu2e {
//-----------------------------------------------------------------------------
MuHitDisplay::MuHitDisplay() : THistModule() {
}

//-----------------------------------------------------------------------------
MuHitDisplay::MuHitDisplay(const art::EDAnalyzer::Table<Config>& config) :
  THistModule                  (config.get_PSet(), config().THistModule.get<fhicl::ParameterSet>(),"MuHitDisplay"),
  _genpCollTag                 (config().genpCollTag()         ),
  _spmcCollTag                 (config().spmcCollTag()         ),
  _caloClusterCollTag          (config().caloClusterCollTag()  ),
  _crvRecoPulseCollTag         (config().crvRecoPulsesCollTag()),
  			        
  _shCollTag                   (config().shCollTag()),
  _comboHitCollTag             (config().chCollTag()),
  _sdCollTag                   (config().sdCollTag()),            // straw digi
  _sdmcCollTag                 (config().sdmcCollTag()),
  _swCollTag                   (config().swCollTag()),           // straw waveformws
  
  _helixSeedCollTag            (config().helixSeedCollTag()),
  _ksfCollTag                  (config().ksfCollTag()),
  _kffCollTag                  (config().kffCollTag()),
  _ctsCollTag                  (config().ctsCollTag()),
  _simpCollTag                 (config().simpCollTag()),
  _timeClusterCollTag          (config().timeClusterCollTag()),
  _phiClusterCollTag           (config().phiClusterCollTag()),
  _caloHitCollTag              (config().caloHitCollTag()),
  _ppTag                       (config().primaryParticleTag()),
  _vdHitsCollTag               (config().vdHitsCollTag()),

  _generatorID                 (config().generatorID()),
  _minSimpMomentum             (config().minSimpMomentum()),
  _maxSimpMomentum             (config().maxSimpMomentum()),

  _showCRVOnly                 (config().showCRVOnly()),
  _showTracks                  (config().showTracks ()),

  _vmConfig                    (config().visManager()),
  _gmConfig                    (config().geoManager())
{

  fApplication = 0;
  fHeaderBlock = new TStnHeaderBlock();
  fVisManager  = TStnVisManager::Instance();
  fGeoManager  = TStnGeoManager::Instance();
  _mcUtils     = std::make_unique<McUtilsToolBase>();
  fTrackID     = new TStnTrackID();
  
  fCrvPulseColl_Right   = new CrvRecoPulseCollection();
  fCrvPulseColl_Left    = new CrvRecoPulseCollection();
  fCrvPulseColl_TopDS   = new CrvRecoPulseCollection();
  fCrvPulseColl_TopTS   = new CrvRecoPulseCollection();
  fCrvPulseColl_Dwnstrm = new CrvRecoPulseCollection();
  fCrvPulseColl_Upstrm  = new CrvRecoPulseCollection();

  // fDar                  = new DoubletAmbigResolver (pset.get<fhicl::ParameterSet>("DoubletAmbigResolver"),0.,0,0);
  // fDarHandle            = new TNamedHandle("DarHandle",fDar);

  // fFolder->Add(fDarHandle);

  fFirstCall            = 1;
  fLastRun              = -1;

  fSimpBlock            = new TSimpBlock;

  TStnVisManager* vm = TStnVisManager::Instance();
//-----------------------------------------------------------------------------
// parse the VM configuration parameters
//-----------------------------------------------------------------------------
  vm->SetDebugLevel(_vmConfig.debugLevel());
    
  stntuple::TEvdPanel::SetDebugLevel(_vmConfig.evdPanelDebugLevel());
  // TStnView::SetDebugLevel (_vmConfig.stnViewDebugLevel ());
    
  int display_straw_digi_mc = _vmConfig.displayStrawDigiMC();
  vm->SetDisplayStrawDigiMC(display_straw_digi_mc);

  int display_straw_hits_xy = _vmConfig.displayStrawHitsXY();
  vm->SetDisplayStrawHitsXY(display_straw_hits_xy);

  float ew_length = _vmConfig.ewLength();
  vm->SetEWLength(ew_length);

  _defaultView           = _vmConfig.defaultView();
  _listOfVisibleStations = _vmConfig.visibleStations();

  float tmin   = _vmConfig.tMin();
  float tmax   = _vmConfig.tMax();
  vm->SetTimeWindow(tmin,tmax);

  float emin = _vmConfig.minEDep();
  float emax = _vmConfig.maxEDep();
  vm->SetMinEDep(emin);
  vm->SetMaxEDep(emax);

}

//-----------------------------------------------------------------------------
MuHitDisplay::~MuHitDisplay() {

  if (fApplication) delete fApplication;

  delete fCrvPulseColl_Right;
  delete fCrvPulseColl_Left;
  delete fCrvPulseColl_TopDS;
  delete fCrvPulseColl_TopTS;
  delete fCrvPulseColl_Dwnstrm;
  delete fCrvPulseColl_Upstrm;

  //  delete fKalDiag;
  // delete fDar;

  delete fSimpBlock;
}

//-----------------------------------------------------------------------------
void MuHitDisplay::beginJob() {

  int    tmp_argc(0);
  char** tmp_argv(0);
//-----------------------------------------------------------------------------
// reuse STNTUPLE initialization of the sim particle list 
// init_block is deleted by the block
//-----------------------------------------------------------------------------
  StntupleInitSimpBlock* init_block = new StntupleInitSimpBlock();

  init_block->SetSimpCollTag       (_simpCollTag);
  init_block->SetShCollTag         (_shCollTag);
  init_block->SetSdmcCollTag       (_sdmcCollTag);
  init_block->SetVDHitsCollTag     (_vdHitsCollTag);
  init_block->SetPrimaryParticleTag(_ppTag);
  init_block->SetMinSimpMomentum   (_minSimpMomentum);         // in MeV
  init_block->SetMaxZ              (1800.);                    // in mm, wrt tracker
  init_block->SetGenProcessID      (-1);
  init_block->SetPdgID             (-1);
  
  fSimpBlock->SetInitBlock(init_block);

  if (!gApplication) {
    fApplication = new TApplication("MuHitDisplay_module", &tmp_argc, tmp_argv);
  }
//-----------------------------------------------------------------------------
// define collection names to be used for initialization
//-----------------------------------------------------------------------------
  TModule::fDump->SetStrawDigiMCCollTag(_sdmcCollTag);
  TModule::beginJob();
}

//-----------------------------------------------------------------------------
void MuHitDisplay::beginRun(const art::Run& Run) {
  
  // mu2e::GeomHandle<mu2e::Tracker> handle;
  // fTracker = &handle.get();

  float mbtime = GlobalConstantsHandle<PhysicsParams>()->getNominalDRPeriod();

  fVisManager->SetMbTime(mbtime);
  fVisManager->SetMinSimpMomentum(_minSimpMomentum);
  fVisManager->SetMaxSimpMomentum(_maxSimpMomentum);
//-----------------------------------------------------------------------------
// if run changes, update tracker panel map - it might have changed
// this should be true for any calibration table
// make sure the tracker is initialized after the tracekr panel map got defined
//-----------------------------------------------------------------------------
  if (fLastRun != (int) Run.run()) {

    art::EventID eid  = art::EventID(Run.run(),1,1);

    ProditionsHandle<TrackerPanelMap> tpm_h;
    fTrkPanelMap = &tpm_h.get(eid);
    fLastRun     = Run.run();
    fGeoManager->SetTrackerPanelMap(fTrkPanelMap);

    fAlignedTracker_h = mu2e::ProditionsHandle<mu2e::Tracker>();
    fAlignedTracker   = fAlignedTracker_h.getPtr(eid).get();

    if (fGeoManager->GetTracker() == nullptr) {
      // stntuple::TEvdTracker* t = new stntuple::TEvdTracker(fTracker);
      stntuple::TEvdTracker* t = new stntuple::TEvdTracker(fAlignedTracker);
      fGeoManager->AddDetector(t);
    }
  }

//-----------------------------------------------------------------------------
// init VisManager - failed to do it in beginJob - what is the right place for doing it?
// get event data and initialize data blocks
//-----------------------------------------------------------------------------
  if (fFirstCall == 1) {
    fFirstCall = 0;
    InitGeoManager();
    InitVisManager();
  }
  
  TModule::beginRun(Run);
}


//-----------------------------------------------------------------------------
void MuHitDisplay::endRun(const art::Run& Run) {
  TStnVisManager* vm = TStnVisManager::Instance();
  vm->SetEvent(nullptr);
//-----------------------------------------------------------------------------
// if a ROOT macro is defined, execute it
//-----------------------------------------------------------------------------
  TModule::endRun(Run);
}


//-----------------------------------------------------------------------------
// initialize the visualization manager
// TStnVisManager is responsible for deleting all nodes created here
//-----------------------------------------------------------------------------
void MuHitDisplay::InitGeoManager() {

  return;
}

//-----------------------------------------------------------------------------
// initialize the visualization manager
// TStnVisManager is responsible for deleting all nodes created here
// assume geometry is already initialized
//-----------------------------------------------------------------------------
void MuHitDisplay::InitVisManager() {
  char oname [100];
  sprintf(oname,"%s:%s",moduleDescription().moduleLabel().data(),"InitVisManager");

  TStnVisManager* vm = TStnVisManager::Instance();
  vm->SetTitleNode(new THeaderVisNode("HeaderVisNode", fHeaderBlock));
//-----------------------------------------------------------------------------
// set station visibility
//-----------------------------------------------------------------------------
  TStnGeoManager* gm = TStnGeoManager::Instance();
  stntuple::TEvdTracker* t  = gm->GetTracker();
  
  for (int i=0; i<18; ++i) {
    int vis = _listOfVisibleStations.at(i);
    t->Station(i)->SetVisible(vis);
  }

  float bfield = _gmConfig.bField();
  gm->SetBField(bfield);
//-----------------------------------------------------------------------------
// init CRV views - 6 of those
//-----------------------------------------------------------------------------
  TCrvView*    view[6];
  TCrvVisNode* node;

  for (int i=0; i<6; i++) {
    view[i] = new TCrvView(i);
    view[i]->SetTimeWindow(0, vm->EWLength());
  }

  node  = new TCrvVisNode("CrvVisNode#0", 0);
  node->SetRecoPulsesCollection(&fCrvPulseColl_Right);
  view[0]->AddNode(node);
    
  node = new TCrvVisNode("CrvVisNode#1", 1);
  node->SetRecoPulsesCollection(&fCrvPulseColl_Left);
  view[1]->AddNode(node);

  node = new TCrvVisNode("CrvVisNode#2", 2);
  node->SetRecoPulsesCollection(&fCrvPulseColl_TopDS);
  view[2]->AddNode(node);

  node = new TCrvVisNode("CrvVisNode#3", 3);
  node->SetRecoPulsesCollection(&fCrvPulseColl_Dwnstrm);
  view[3]->AddNode(node);

  node = new TCrvVisNode("CrvVisNode#4", 4);
  node->SetRecoPulsesCollection(&fCrvPulseColl_Upstrm);
  view[4]->AddNode(node);

    // '8' is not a typo...
  node = new TCrvVisNode("CrvVisNode#5", 8);
  node->SetRecoPulsesCollection(&fCrvPulseColl_TopTS);
  view[5]->AddNode(node);

  for (int i=0; i<6; i++) {
    vm->AddView(view[i]);
  }
//-----------------------------------------------------------------------------
// do the geometry
//-----------------------------------------------------------------------------
  art::ServiceHandle<mu2e::GeometryService> geom;

  mu2e::GeomHandle<mu2e::DiskCalorimeter> dc_handle;
  const mu2e::DiskCalorimeter* dc = dc_handle.get();
//-----------------------------------------------------------------------------
// TCalVisNode: calorimeter, CaloCrystalHits and CaloCrystals
// cal_node[0] : first  disk
// cal_node[1] : second disk
//-----------------------------------------------------------------------------
  TCalView*         cal_view;
  TCalVisNode*      cal_node[2];

  cal_view    = new TCalView(0);
  cal_node[0] = new TCalVisNode("CalVisNode#0", &dc->disk(0), 0);
  cal_node[0]->SetListOfClusters(&_caloClusterColl);
  cal_node[0]->SetListOfCrystalHits(&_caloHitColl);
  cal_view->AddNode(cal_node[0]);
  vm->AddView(cal_view);
  
  cal_view    = new TCalView(1);
  cal_node[1] = new TCalVisNode("CalVisNode#1", &dc->disk(1), 1);
  cal_node[1]->SetListOfClusters   (&_caloClusterColl);
  cal_node[1]->SetListOfCrystalHits(&_caloHitColl);
  cal_view->AddNode(cal_node[1]);
  vm->AddView(cal_view);
//-----------------------------------------------------------------------------
// TrkVisNode: tracker, tracks, cosmic tracks and straw hits
// add tracker node to two views - RZ and XY, also - to the panel VRZ
// tracks are KalSeed's
//-----------------------------------------------------------------------------
  TTrkVisNode* trk_vis_node = new TTrkVisNode ("TrkVisNode", fAlignedTracker, NULL);

  trk_vis_node->SetShCollTag       (_shCollTag      );
  trk_vis_node->SetChCollTag       (_comboHitCollTag);
  trk_vis_node->SetKsCollTag       (_kffCollTag     );
  trk_vis_node->SetCtsCollTag      (_ctsCollTag     );

  trk_vis_node->SetSdmcCollTag     (_sdmcCollTag    );
  trk_vis_node->SetSimpColl        (&_simpColl      );
  trk_vis_node->SetSimpCollTag     (_simpCollTag    );
  trk_vis_node->SetSpmcColl        (&_spmcColl      );
  trk_vis_node->SetSwColl          (&_swColl        );
//-----------------------------------------------------------------------------
// SimpBlock is initialized in the module, a node references it via the pointer
//-----------------------------------------------------------------------------
  trk_vis_node->SetSimpBlock       (fSimpBlock      );
//-----------------------------------------------------------------------------
// HelixVisNode: one helix collection - lets see how it plays out
// add helix node to only one view - XY
//-----------------------------------------------------------------------------
  TEvdHelixVisNode* hnode = new TEvdHelixVisNode ("HelixVisNode", NULL);
  hnode->SetHelixSeedCollTag(_helixSeedCollTag);
  hnode->SetSdmcCollTag     (_sdmcCollTag);
  hnode->SetShCollTag       (_shCollTag );
//-----------------------------------------------------------------------------
// TimeClusterVisNode: one time collection 
// to begin with, add timecluster node to only one view - TZ
//-----------------------------------------------------------------------------
  TEvdTimeClusterVisNode* tc_node = new TEvdTimeClusterVisNode ("TimeClusterVisNode", NULL);
  tc_node->SetTcCollTag  (_timeClusterCollTag);
  tc_node->SetPcCollTag  (_phiClusterCollTag );
  tc_node->SetChCollTag  (_comboHitCollTag   );
  tc_node->SetSschCollTag(_shCollTag   );
  tc_node->SetSdmcCollTag(_sdmcCollTag       );
//-----------------------------------------------------------------------------
// nodes are defined, now come views
// 1. XY view : tracker + calorimeter + time clusters
//-----------------------------------------------------------------------------
  TStnView* vxy = new TStnView(TStnVisManager::kXY,-1,"XYView","XY View");
  vxy->AddNode(trk_vis_node);
  vxy->AddNode(hnode);
  vxy->AddNode(cal_node[0]);
  vxy->AddNode(cal_node[1]);
  vxy->AddNode(tc_node);
  vm->AddView(vxy);
//-----------------------------------------------------------------------------
// 2. RZ view : tracker only, so far
//-----------------------------------------------------------------------------
  TStnView* vrz = new TStnView(TStnVisManager::kRZ,-1,"RZView","RZ View");
  vrz->AddNode(trk_vis_node);
  vm->AddView(vrz);
//-----------------------------------------------------------------------------
// 3. TZ view : TrkNode + TimeClusterNode
//-----------------------------------------------------------------------------
  TStnView* vtz = new TStnView(TStnVisManager::kTZ,-1,"TZView","TZ View");
  vtz->AddNode(trk_vis_node);
  vtz->AddNode(tc_node);
  vm->AddView(vtz);
//-----------------------------------------------------------------------------
// 4. TZ view : TrkNode - for now, hits only 
//-----------------------------------------------------------------------------
  TStnView* vphiz = new TStnView(TStnVisManager::kPhiZ,-1,"PhiZView","PhiZ View");
  vphiz->AddNode(trk_vis_node);
  vm->AddView(vphiz);
//-----------------------------------------------------------------------------
// 5. VST XY view : TTrkNode only, so far
//-----------------------------------------------------------------------------
  TStnView* vst = new TStnView(TStnVisManager::kVST,-1,"VSTView","VST View");
  vst->AddNode(trk_vis_node);
  vm->AddView(vst);
//-----------------------------------------------------------------------------
// VRZ: "VST RZ" view - one per panel, each VRZ view shows only one panel
//-----------------------------------------------------------------------------
  const int nfaces(4), np_face(3);
  
  stntuple::TEvdTracker* evd_tracker = gm->GetTracker();

  int ns = 1; // fTracker->nStations();

  for (int is=0; is<ns; ++is) {
    for (int face=0; face<nfaces; ++face) {
      for (int ip=0; ip<np_face; ++ip) {
        int ipln, ipnl;
        stntuple::TEvdTracker::ConvertPanelGeoIndices(is,face,ip,ipln,ipnl);

        int geo_id  = 12*is+6*ipln+ipnl;
        TStnView* v = new TStnView(TStnVisManager::kVRZ,geo_id,
                                   "VRZView",
                                   Form("panel %02i:%i",2*is+ipln,ipnl));

        stntuple::TEvdPanel* evd_panel = evd_tracker->Station(is)->Plane(ipln)->Panel(ipnl);
        const mu2e::Panel*   panel     = evd_panel->GetMu2ePanel();
        v->SetMother((void*) panel);
        //        v->CloneCombiTrans(evd_panel->GetCombiTrans());
//-----------------------------------------------------------------------------
// create VisNode for this panel and, together with the TrkVisNode, add it to the view
//-----------------------------------------------------------------------------
        stntuple::TEvdPanelVisNode* vp_node = new stntuple::TEvdPanelVisNode (Form("EvdPanel_VN_%02i_%i_%i",is,ipln,ipnl),evd_panel);
        v->AddNode(vp_node);
        v->AddNode(trk_vis_node);
//-----------------------------------------------------------------------------
// finally, add the view to the VM's list of views
//-----------------------------------------------------------------------------
        vm->AddView(v);
      }
    }
  }
  
//-----------------------------------------------------------------------------
// upon startup, open either an XY or a VST view
//-----------------------------------------------------------------------------
  if      (_defaultView == "xy" ) vm->OpenTrkXYView();
  else if (_defaultView == "vst") vm->OpenVSTView  ();
}

//-----------------------------------------------------------------------------
// get data from the event record
//-----------------------------------------------------------------------------
int MuHitDisplay::getData(const art::Event* ArtEvent) {
  char oname[100];
  sprintf(oname,"%s::%s",moduleDescription().moduleLabel().data(),"getData");
//-----------------------------------------------------------------------------
//  CRV pulse information
//-----------------------------------------------------------------------------
  art::Handle<CrvRecoPulseCollection> pulsesHandle;
  ArtEvent->getByLabel(_crvRecoPulseCollTag, pulsesHandle);
    
  if (pulsesHandle.isValid()) {
    const mu2e::CrvRecoPulseCollection* fCrvPulseColl = (CrvRecoPulseCollection*) pulsesHandle.product();
      
    // Clear the map pointers in preperation to (re)fill them with new information
    fCrvPulseColl_Right->clear();
    fCrvPulseColl_Left->clear();
    fCrvPulseColl_TopDS->clear();
    fCrvPulseColl_TopTS->clear();
    fCrvPulseColl_Dwnstrm->clear();
    fCrvPulseColl_Upstrm->clear();
    printf(">>> Section-collections cleared\n");
      
    mu2e::GeomHandle<mu2e::CosmicRayShield> CRS;
//-----------------------------------------------------------------------------
// Loop over the RecoPulses in the collection and sort each pulse/bar 
// into its appropriate section
//-----------------------------------------------------------------------------
    int    crvCollSize = fCrvPulseColl->size();
    for (int ic=0; ic < crvCollSize; ++ic) {
      mu2e::CrvRecoPulse                   icprc       = fCrvPulseColl->at(ic);
      const mu2e::CRSScintillatorBarIndex &CRVBarIndex = icprc.GetScintillatorBarIndex();
      
      int shield = CRS->getBar(CRVBarIndex).id().getShieldNumber();
      
      if      (shield == 0) fCrvPulseColl_Right->  push_back(icprc);
      else if (shield == 1) fCrvPulseColl_Left->   push_back(icprc);
      else if (shield == 2) fCrvPulseColl_TopDS->  push_back(icprc);
      else if (shield == 3) fCrvPulseColl_Dwnstrm->push_back(icprc);
      else if (shield == 4) fCrvPulseColl_Upstrm-> push_back(icprc);
      else if (shield == 8) fCrvPulseColl_TopTS->  push_back(icprc);
    }
								
    printf(">>> Section-collections filled\n");
  }
  else {
    printf(">>> [MuHitDisplay::%s] WARNING: CrvRecoPulsesCollection by %s is missing. CONTINUE.\n",
	   __func__, _crvRecoPulseCollTag.encode().data());
  }
    
  if (_showCRVOnly) { //If only displaying the CRV, skip everything else
    printf("!!!!! ONLY SHOWING THE CRV\n");
    return 0;
  }
  else {
//-----------------------------------------------------------------------------
//  MC truth - gen particles
//-----------------------------------------------------------------------------
    art::Handle<GenParticleCollection> gensHandle;
    ArtEvent->getByLabel(_genpCollTag, gensHandle);

    if (gensHandle.isValid()) _genpColl = gensHandle.product();
    else {
      _genpColl = 0;
      printf(">>> [%s] WARNING: GenParticleCollection by %s is missing. CONTINUE\n",
	     oname, _genpCollTag.encode().data());
    }
//-----------------------------------------------------------------------------
//  StepPointMCs - on virtual detectors
//-----------------------------------------------------------------------------
    art::Handle<StepPointMCCollection> vdStepsHandle;
    ArtEvent->getByLabel(_spmcCollTag, vdStepsHandle);
    
    if (vdStepsHandle.isValid()) _spmcColl = vdStepsHandle.product();
    else                         _spmcColl = NULL;
//-----------------------------------------------------------------------------
// SimParticle's
//-----------------------------------------------------------------------------
    art::Handle<mu2e::SimParticleCollection> simpHandle;
    ArtEvent->getByLabel(_simpCollTag, simpHandle);

    if (simpHandle.isValid()) _simpColl = simpHandle.product();
    else                      _simpColl = NULL;
//-----------------------------------------------------------------------------
//  straw hit information
//-----------------------------------------------------------------------------
    art::Handle<StrawDigiCollection> sdcH;
    ArtEvent->getByLabel(_sdCollTag, sdcH);
    if (sdcH.isValid()) _sdColl = sdcH.product();
    else {
      printf(">>> [%s] WARNING: StrawDigiCollection by %s is missing.\n",
	     oname, _shCollTag.encode().data());
      _sdColl = nullptr;
    }

    art::Handle<StrawDigiADCWaveformCollection> swcH;
    ArtEvent->getByLabel(_swCollTag, swcH);
    if (swcH.isValid()) _swColl = swcH.product();
    else {
      printf(">>> [%s] WARNING: StrawDigiADCWaveformCollection by %s is missing.\n",
	     oname, _swCollTag.encode().data());
      _swColl = nullptr;
    }
    
    art::Handle<StrawDigiMCCollection> sdmccH;
    ArtEvent->getByLabel(_sdmcCollTag, sdmccH);
    if (sdmccH.isValid()) {
      _sdmcColl = sdmccH.product();
      if (_sdmcColl->size() <= 0) {
	printf(">>> [%s] WARNING:StrawDigiMCCollection by %s has zero length. CONTINUE\n",
	       oname, _sdmcCollTag.encode().data());
      }
    }
    else {
      printf(">>> [%s] WARNING: mu2e::StrawDigiMCCollection by %s is missing\n",
	     oname, _sdmcCollTag.encode().data());
      _sdmcColl = nullptr;
    }
//-----------------------------------------------------------------------------
// calorimeter hit data
//-----------------------------------------------------------------------------
    art::Handle<CaloHitCollection> calohit_ch;
    ArtEvent->getByLabel(_caloHitCollTag,calohit_ch);
    
    if (calohit_ch.isValid()) {
      _caloHitColl = (CaloHitCollection*) calohit_ch.product();
    }
    else {
      _caloHitColl = NULL;
      printf(">>> [%s] WARNING: CaloHitCollection by %s is missing.\n",
	     oname, _caloHitCollTag.encode().data());
    }
//-----------------------------------------------------------------------------
// calorimeter cluster data
//-----------------------------------------------------------------------------
    art::Handle<CaloClusterCollection> calocluster_ch;
    ArtEvent->getByLabel(_caloClusterCollTag,calocluster_ch);
    
    if (calocluster_ch.isValid()) {
      _caloClusterColl = calocluster_ch.product();
    }
    else {
      _caloClusterColl = NULL;
      printf(">>> [%s] WARNING: CaloClusterCollection by %s is missing.\n",
	     oname, _caloClusterCollTag.encode().data());
    }
//-----------------------------------------------------------------------------
// finally, TSimpBlock. The second parameter, Mode, is not used
//-----------------------------------------------------------------------------
    fSimpBlock->Init((art::Event*)ArtEvent,0);
    
    return 0;
  }
}

//-----------------------------------------------------------------------------
void MuHitDisplay::analyze(const art::Event& ArtEvent) {
  //    const char* oname = "MuHitDisplay::analyze";

  printf("[MuHitDisplay::%s] RUN: %10i EVENT: %10i\n", __func__, ArtEvent.run(), ArtEvent.event());

  getData(&ArtEvent);
  
  fVisManager->SetEvent(&ArtEvent);
  fVisManager->DisplayEvent();
//-----------------------------------------------------------------------------
// go into interactive mode, 
// fInteractiveMode = 1 : stop after each event (event display mode)
// fInteractiveMode = 2 : stop only in the end of run, till '.q' is pressed
//-----------------------------------------------------------------------------
  TModule::analyze(ArtEvent);
  return;
} 

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int MuHitDisplay::getCRVSection(int shieldNumber) {
  int CRVSection = -1;
  switch (shieldNumber) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:  CRVSection = 0; break;  //R
  case 6:
  case 7:
  case 8:  CRVSection = 1; break;  //L
  case 9:  CRVSection = 8; break;  //TS T
  case 10:
  case 11:
  case 12: CRVSection = 2; break;  //T
  case 13: CRVSection = 3; break;  //D
  case 14: CRVSection = 4; break;  //U
  case 15: CRVSection = 5; break;  //CU
  case 16: CRVSection = 6; break;  //CD
  case 17: CRVSection = 7; break;  //CT
  }
  return CRVSection;
}

}

using mu2e::MuHitDisplay;
DEFINE_ART_MODULE(MuHitDisplay)
