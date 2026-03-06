//

#include "Stntuple/print/TAnaDump.hh"
#include "Stntuple/print/Stntuple_print_functions.hh"
#include "Stntuple/geom/TCrvNumerology.hh"

#include "TROOT.h"
#include "TVector2.h"
#include "CLHEP/Vector/ThreeVector.h"

#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/Selector.h"

#include "Offline/GeometryService/inc/GeometryService.hh"
#include "Offline/GeometryService/inc/GeomHandle.hh"

#include "Offline/TrackerGeom/inc/Tracker.hh"
#include "Offline/CalorimeterGeom/inc/DiskCalorimeter.hh"
#include "Offline/CalorimeterGeom/inc/Calorimeter.hh"

#include "Offline/RecoDataProducts/inc/CaloCluster.hh"
#include "Offline/RecoDataProducts/inc/CaloProtoCluster.hh"
#include "Offline/RecoDataProducts/inc/CaloHit.hh"

#include "Offline/RecoDataProducts/inc/CrvRecoPulse.hh"

#include "Offline/RecoDataProducts/inc/CrvDigi.hh"

#include "Offline/RecoDataProducts/inc/CrvCoincidence.hh"
#include "Offline/RecoDataProducts/inc/CrvCoincidenceCluster.hh"

#include "Offline/RecoDataProducts/inc/StrawHit.hh"
#include "Offline/RecoDataProducts/inc/KalSeed.hh"
#include "Offline/RecoDataProducts/inc/HelixSeed.hh"
#include "Offline/RecoDataProducts/inc/ComboHit.hh"
#include "Offline/RecoDataProducts/inc/TrkStrawHitSeed.hh"
#include "Offline/RecoDataProducts/inc/TimeCluster.hh"

#include "Offline/RecoDataProducts/inc/CaloDigi.hh"
#include "Offline/RecoDataProducts/inc/CaloRecoDigi.hh"

#include "Offline/MCDataProducts/inc/GenParticle.hh"
#include "Offline/MCDataProducts/inc/SimParticle.hh"
#include "Offline/MCDataProducts/inc/StrawGasStep.hh"
#include "Offline/MCDataProducts/inc/StepPointMC.hh"
#include "Offline/MCDataProducts/inc/PhysicalVolumeInfo.hh"
#include "Offline/MCDataProducts/inc/PhysicalVolumeInfoMultiCollection.hh"



// #include "Offline/RecoDataProducts/inc/KalRepPtrCollection.hh"

// #include "Offline/RecoDataProducts/inc/TrkToCaloExtrapol.hh"
// #include "Offline/RecoDataProducts/inc/TrkCaloIntersect.hh"
// #include "Offline/RecoDataProducts/inc/TrackClusterMatch.hh"
#include "Offline/CommonMC/inc/TrkMCTools.hh"

#include "Offline/Mu2eUtilities/inc/LsqSums4.hh"

#include "Stntuple/base/TNamedHandle.hh"

#include "Offline/CommonMC/inc/TrkMCTools.hh"

// BTRK (BaBar) includes
#include "BTrk/BbrGeom/HepPoint.h"
#include "BTrk/BbrGeom/TrkLineTraj.hh"
#include "BTrk/TrkBase/TrkPoca.hh"
#include "BTrk/KalmanTrack/KalHit.hh"
#include "BTrk/KalmanTrack/KalRep.hh"
#include "BTrk/TrkBase/HelixParams.hh"
#include "BTrk/ProbTools/ChisqConsistency.hh"

// #include "Offline/TrkReco/inc/TrkPrintUtils.hh"

#include "CLHEP/Geometry/Point3D.h"

using namespace std;
// typedef HepGeom::Point3D<double> HepPoint;

ClassImp(TAnaDump)

TAnaDump* TAnaDump::fgInstance = 0;

//-----------------------------------------------------------------------------
// WeightMode = 1 is for XY chi2 , WeightMode = 0 is for Phi-z chi2
//-----------------------------------------------------------------------------
double TAnaDump::evalWeight(const mu2e::ComboHit* Hit   ,
			    CLHEP::Hep3Vector&    StrawDir ,
			    CLHEP::Hep3Vector&    HelCenter, 
			    double                Radius   ,
			    int                   WeightMode,
			    fhicl::ParameterSet const& Pset) {
  
  // double    rs(2.5);   // straw radius, mm
  // double    ew(30.0);  // assumed resolution along the wire, mm
  double    transErr = 5./sqrt(12.);
  //scale the error based on the number of the strawHits that are within teh ComboHit
  if (Hit->nStrawHits() > 1) transErr *= 1.5;
  double    transErr2 = transErr*transErr;

  double x  = Hit->pos().x();
  double y  = Hit->pos().y();
  double dx = x-HelCenter.x();
  double dy = y-HelCenter.y();
  
  double costh  = (dx*StrawDir.x()+dy*StrawDir.y())/sqrt(dx*dx+dy*dy);
  double costh2 = costh*costh;
  double sinth2 = 1-costh2;
  
  double wt(0), wtXY(1), wtPhiZ(1);

  //  fhicl::ParameterSet const& pset = helix_handle.provenance()->parameterSet();
  string                module     = Pset.get<string>("module_type");
  
  if ( module == "CalHelixFinder"){
    fhicl::ParameterSet const& psetHelFit = Pset.get<fhicl::ParameterSet>("HelixFinderAlg", fhicl::ParameterSet());
    wtXY   = psetHelFit.get<double>("weightXY");
    wtPhiZ = psetHelFit.get<double>("weightZPhi");
  }
                                            //scale the weight for having chi2/ndof distribution peaking at 1
  double werr2  = Hit->wireRes()*Hit->wireRes();
  if ( WeightMode == 1){//XY-Fit
    // double e2     = ew*ew*sinth2+rs*rs*costh*costh;
    double e2     = werr2*sinth2+transErr2*costh2;
    wt  = 1./e2;
    wt *= wtXY;
  } else if (WeightMode ==0 ){//Phi-Z Fit
    // double e2     = ew*ew*costh*costh+rs*rs*sinth2;
    double e2     = werr2*costh2+transErr2*sinth2;
    wt     = Radius*Radius/e2;
    wt    *= wtPhiZ;
  }
  
  return wt;
}

//-----------------------------------------------------------------------------
TAnaDump::TAnaDump(const fhicl::ParameterSet* PSet) {

  fEvent                  = nullptr;
  fListOfObjects          = new TObjArray();
  fFlagBgrHitsModuleLabel = "FlagBkgHits";
  fSdmcCollTag            = "compressDigiMCs";

  // _printUtils = new mu2e::TrkPrintUtils(PSet->get<fhicl::ParameterSet>("printUtils",fhicl::ParameterSet()));
}

// //-----------------------------------------------------------------------------
// TAnaDump::TAnaDump(const fhicl::Table<Config>& config) {

//   fEvent                  = nullptr;
//   fListOfObjects          = new TObjArray();
//   fFlagBgrHitsModuleLabel = "FlagBkgHits";
//   fSdmcCollTag            = "compressDigiMCs";
//   _printUtils             = new mu2e::TrkPrintUtils(config().printUtils());
// }

//------------------------------------------------------------------------------
TAnaDump* TAnaDump::Instance(const fhicl::ParameterSet* PSet) {
  static TAnaDump::Cleaner cleaner;

  if  (! fgInstance) fgInstance  = new TAnaDump(PSet);
  return fgInstance;
}


//______________________________________________________________________________
TAnaDump::~TAnaDump() {
  fListOfObjects->Delete();
  delete fListOfObjects;
  // delete _printUtils;
}

//------------------------------------------------------------------------------
TAnaDump::Cleaner::Cleaner() {
}

//------------------------------------------------------------------------------
  TAnaDump::Cleaner::~Cleaner() {
    if (TAnaDump::fgInstance) {
      delete TAnaDump::fgInstance;
      TAnaDump::fgInstance = 0;
    }
  }


//-----------------------------------------------------------------------------
void TAnaDump::AddObject(const char* Name, void* Object) {
  TNamedHandle* h = new TNamedHandle(Name,Object);
  fListOfObjects->Add(h);
}

//-----------------------------------------------------------------------------
void* TAnaDump::FindNamedObject(const char* Name) {
  void* o(NULL);

  TNamedHandle* h = (TNamedHandle*) fListOfObjects->FindObject(Name);
  if (h != NULL) {
    o = h->Object();
  }
  return o;
}

//-----------------------------------------------------------------------------
// print position of the cluster in the tracker system
//-----------------------------------------------------------------------------
void TAnaDump::printCaloCluster(const mu2e::CaloCluster* Cl, 
				const char* Opt, 
				const mu2e::CaloHitMCTruthAssn* CaloHitTruth) {
  int row, col;
  TString opt = Opt;

  art::ServiceHandle<mu2e::GeometryService> geom;
  mu2e::GeomHandle  <mu2e::Calorimeter>     cal;
  CLHEP::Hep3Vector        gpos, tpos;

  if ((opt == "") || (opt.Index("banner") >= 0)) {
    printf("-----------------------------------------------------------------------------------------------");
    printf("-------------------------------\n");
    printf(" Row Col Address        Disk Parent  NC   Energy   Time       X(loc)     Y(loc)   Z(loc)");
    printf("        X          Y          Z\n");
    printf("-----------------------------------------------------------------------------------------------");
    printf("-------------------------------\n");
  }
 
  if ((opt == "") || (opt.Index("data") >= 0)) {
    row = -1; // Cl->cogRow();
    col = -1; // Cl->cogColumn();
    
    const mu2e::CaloHitPtrVector caloClusterHits = Cl->caloHitsPtrVector();
    int nh = caloClusterHits.size();

    if ((row < 0) || (row > 9999)) row = -99;
    if ((col < 0) || (col > 9999)) col = -99;
//-----------------------------------------------------------------------------
// transform cluster coordinates to the tracker coordiante system
//-----------------------------------------------------------------------------
    gpos = cal->geomUtil().diskToMu2e(Cl->diskID(),Cl->cog3Vector());
    tpos = cal->geomUtil().mu2eToTracker(gpos);

    printf(" %3i %3i %-16p %2i %6i %3i %8.3f %8.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f\n",
	   row, col,
	   static_cast<const void*>(Cl),
	   Cl->diskID(),
	   -999, 
	   nh,
	   Cl->energyDep(),
	   Cl->time(),
	   Cl->cog3Vector().x(),
	   Cl->cog3Vector().y(),
	   Cl->cog3Vector().z(),
	   tpos.x(),
	   tpos.y(),
	   tpos.z() 
	   );
  }
  
  if (opt.Index("hits") >= 0) {

    const mu2e::Crystal           *cr;
    const CLHEP::Hep3Vector       *pos;
    // int  iz, ir;
//-----------------------------------------------------------------------------
// print individual crystals in local vane coordinate system
//-----------------------------------------------------------------------------
    const mu2e::CaloHitPtrVector caloClusterHits = Cl->caloHitsPtrVector();
    int nh = caloClusterHits.size();
    
    //    mu2e::Calorimeter const & calo = *(mu2e::GeomHandle<mu2e::Calorimeter>());

    printf("-----------------------------------------------------------------------------------------------");
    printf("-------------------------------\n");
    printf("    Id       time      Gen-Code     ID   PDG  PDG(M)      energy       X(loc)     Y(loc)   Z(loc)    energy-MC     nSimP\n");
    printf("-----------------------------------------------------------------------------------------------");
    printf("-------------------------------\n");
   
    for (int i=0; i<nh; i++) {
      const mu2e::CaloHit* hit = &(*caloClusterHits.at(i));
      int id = hit->crystalID();
      
      cr = &cal->crystal(id);

      pos = &cr->localPosition();
  
      printf("TAnaDump::printCaloCluster ERROR: CrystalContentMC is gone, FIXIT\n");

      //      mu2e::CrystalContentMC contentMC(calo, *CaloHitTruth, *hit);
      double                 simMaxEdep(0);
      int                    simPDGId(9999), simPDGM(9999), simCreationCode(9999), /*simTime(9999),*/ simID(9999);

      //search the simParticle that gave the largest energy contribution

      int  nSimPart(0);

      // for (const auto& contentMap : contentMC.simContentMap() )	{	       
      // 	art::Ptr<mu2e::SimParticle> sim  = contentMap.first;
      // 	mu2e::CaloContentSim        data = contentMap.second;
               
      // 	auto parent(sim);
      // 	while ( parent->hasParent()) parent = parent->parent();               
               
      // 	if (data.edep() > simMaxEdep){
      // 	  simMaxEdep = data.edep();
	    
      // 	  simID            = sim->id().asInt();
      // 	  simPDGId         = sim->pdgId();
      // 	  simCreationCode  = parent->creationCode();
      // 	  // simTime          = data.time();
      // 	  simPDGM          = parent->pdgId(); 
      // 	}
      // 	++nSimPart;
      // }
      
      printf("%6i   %10.3f %8i %8i %5i %5i    %10.3f   %10.3f %10.3f %10.3f %10.3f %8i\n",
	     id,
	     hit->time(),
	     simCreationCode, simID,simPDGId,simPDGM,
	     // iz,ir,
	     hit->energyDep(),
	     pos->x(),
	     pos->y(),
	     pos->z(),
	     simMaxEdep,
	     nSimPart
	     );
    }
  }
}


//-----------------------------------------------------------------------------
void TAnaDump::printCaloClusterCollection(const char* ModuleLabel, 
					  const char* ProductName,
					  const char* ProcessName,
					  double      Emin,
					  int         HitOpt,
					  const char* MCModuleLabel) {

  printf(">>>> ModuleLabel = %s\n",ModuleLabel);

  //data about hits in the calorimeter crystals

  art::Handle<mu2e::CaloClusterCollection> handle;
  const mu2e::CaloClusterCollection* caloCluster;

  art::Handle<mu2e::CaloHitMCTruthAssn> caloHitTruthHandle;
  const mu2e::CaloHitMCTruthAssn* caloHitTruth(0);
  
  if (ProductName[0] != 0) {
    art::Selector  selector(art::ProductInstanceNameSelector(ProductName) &&
			    art::ProcessNameSelector(ProcessName)         && 
			    art::ModuleLabelSelector(ModuleLabel)            );
    fEvent->get(selector,handle);
    art::Selector  selectorMC(art::ProductInstanceNameSelector(ProductName) &&
			      art::ProcessNameSelector(ProcessName)         && 
			      art::ModuleLabelSelector(MCModuleLabel)            );
   
    fEvent->get(selectorMC,caloHitTruthHandle);
  }
  else {
    art::Selector  selector(art::ProcessNameSelector(ProcessName)         && 
			    art::ModuleLabelSelector(ModuleLabel)            );
    fEvent->get(selector,handle);
    art::Selector  selectorMC(art::ProcessNameSelector(ProcessName)         && 
			      art::ModuleLabelSelector(MCModuleLabel)            );
    fEvent->get(selectorMC,caloHitTruthHandle);
  }
//-----------------------------------------------------------------------------
// make sure collection exists
//-----------------------------------------------------------------------------
  if (! handle.isValid()) {
    printf("TAnaDump::printCaloClusterCollection: no CaloClusterCollection ");
    printf("for module %s and ProductName=%s found, BAIL OUT\n",
	   ModuleLabel,ProductName);
    return;
  }

  caloCluster = handle.product();

  int ncl = caloCluster->size();

  if (caloHitTruthHandle.isValid()) caloHitTruth = caloHitTruthHandle.product();

  const mu2e::CaloCluster* cl;


  int banner_printed = 0;
  for (int i=0; i<ncl; i++) {
    cl = &caloCluster->at(i);
    if (cl->energyDep() < Emin )    continue;
    
    if ( (banner_printed == 0) && (HitOpt==0) ) {
      printCaloCluster(cl, "banner");
      banner_printed = 1;
    }
    if(HitOpt==0) {
      printCaloCluster(cl,"data");
    } else{
      printCaloCluster(cl,"banner+data+hits",caloHitTruth);
    }
  }
 
}


//-----------------------------------------------------------------------------
void TAnaDump::printCaloProtoCluster(const mu2e::CaloProtoCluster* Cluster, const char* Opt) {

  TString opt = Opt;

  int section_id(-1), iz, ir;

  const mu2e::Calorimeter       * cal(NULL);
  const mu2e::Crystal           *cr;
  const CLHEP::Hep3Vector       *pos;

  art::ServiceHandle<mu2e::GeometryService> geom;
  mu2e::GeomHandle  <mu2e::Calorimeter>     cg;

  cal = cg.get();

  if ((opt == "") || (opt == "banner")) {
    printf("-----------------------------------------------------------------------------------------------------\n");
    printf("       Address  SectionID  IsSplit  NC    Time    Energy      \n");
    printf("-----------------------------------------------------------------------------------------------------\n");
  }
 
  const mu2e::CaloHitPtrVector caloClusterHits = Cluster->caloHitsPtrVector();
  int nh = caloClusterHits.size();

  if ((opt == "") || (opt.Index("data") >= 0)) {

    printf("%16p  %3i %5i %5i %10.3f %10.3f\n",
	   static_cast<const void*>(Cluster),
	   section_id,
	   nh,
	   Cluster->isSplit(),
	   Cluster->time(),
	   Cluster->energyDep()
	   ); 
  }
  
  if (opt.Index("hits") >= 0) {
//-----------------------------------------------------------------------------
// print individual crystals in local vane coordinate system
//-----------------------------------------------------------------------------
    for (int i=0; i<nh; i++) {
      const mu2e::CaloHit* hit = &(*caloClusterHits.at(i));
      int id = hit->crystalID();
      
      cr  = &cal->crystal(id);
      pos = &cr->localPosition();
      iz  = -1;
      ir  = -1;
      
      printf("%6i     %10.3f %5i %5i %8.3f %10.3f %10.3f %10.3f %10.3f\n",
	     id,
	     hit->time(),
	     iz,ir,
	     hit->energyDep(),
	     pos->x(),
	     pos->y(),
	     pos->z(),
	     hit->energyDepTot()
	     );
    }
  }
}


//-----------------------------------------------------------------------------
void TAnaDump::printCaloProtoClusterCollection(const char* ModuleLabel, 
					       const char* ProductName,
					       const char* ProcessName) {

  art::Handle<mu2e::CaloProtoClusterCollection> handle;
  const mu2e::CaloProtoClusterCollection       *coll;
  const mu2e::CaloProtoCluster                 *cluster;

  int banner_printed(0), nclusters;

  if (ProductName[0] != 0) {
    art::Selector  selector(art::ProductInstanceNameSelector(ProductName) &&
			    art::ProcessNameSelector(ProcessName)         && 
			    art::ModuleLabelSelector(ModuleLabel)            );
    fEvent->get(selector,handle);
  }
  else {
    art::Selector  selector(art::ProcessNameSelector(ProcessName)         && 
			    art::ModuleLabelSelector(ModuleLabel)            );
    fEvent->get(selector,handle);
  }
//-----------------------------------------------------------------------------
// make sure collection exists
//-----------------------------------------------------------------------------
  if (! handle.isValid()) {
    printf("TAnaDump::printCaloProtoClusterCollection: no CaloProtoClusterCollection ");
    printf("for module %s and ProductName=%s found, BAIL OUT\n",
	   ModuleLabel,ProductName);
    return;
  }

  coll      = handle.product();
  nclusters = coll->size();

  for (int i=0; i<nclusters; i++) {
    cluster = &coll->at(i);
    if (banner_printed == 0) {
      printCaloProtoCluster(cluster, "banner");
      banner_printed = 1;
    }
    printCaloProtoCluster(cluster,"data");
  }
}

//-----------------------------------------------------------------------------
// CRV
//-----------------------------------------------------------------------------
void TAnaDump::printCrvCoincidence(const mu2e::CrvCoincidence* Coin,
				   const char*                 Opt ) {
  TString opt = Opt;

  const std::vector<art::Ptr<mu2e::CrvRecoPulse>>* list_of_pulses = &Coin->GetCrvRecoPulses();

  int sector      = Coin->GetCrvSectorType();
  int np          = list_of_pulses->size();

  printf("---------------------------------------------------------------------\n");
  printf("Coinc Addr: %-16p Sector: %5i N(pulses): %5i\n",static_cast<const void*>(Coin), sector, np);

  const mu2e::CrvRecoPulse* pulse(NULL);
  printCrvRecoPulse(pulse, "banner");
  for (int i=0; i<np; i++) {
    pulse = list_of_pulses->at(i).get();
    printCrvRecoPulse(pulse, "data");
  }
  
  if (opt.Index("hits") >= 0) {
  }
}

//-----------------------------------------------------------------------------
void TAnaDump::printCrvCoincidenceCollection(const char* ModuleLabel, 
					     const char* ProductName,
					     const char* ProcessName) {

  art::Handle<mu2e::CrvCoincidenceCollection> handle;
  const mu2e::CrvCoincidenceCollection*       coinColl;

  if (ProductName[0] != 0) {
    art::Selector  selector(art::ProductInstanceNameSelector(ProductName) &&
			    art::ProcessNameSelector(ProcessName)         && 
			    art::ModuleLabelSelector(ModuleLabel)            );
    fEvent->get(selector,handle);
  }
  else {
    art::Selector  selector(art::ProcessNameSelector(ProcessName)         && 
			    art::ModuleLabelSelector(ModuleLabel)            );
    fEvent->get(selector,handle);
  }
//-----------------------------------------------------------------------------
// make sure collection exists
//-----------------------------------------------------------------------------
  if (! handle.isValid()) {
    printf("TAnaDump::printCrvCoincidenceCollection: no CrvCoincidenceCollection ");
    printf("for module %s and ProductName=%s found, BAIL OUT\n",
	   ModuleLabel,ProductName);
    return;
  }

  coinColl = handle.product();

  int ncoin = coinColl->size();

  printf(">>>> ModuleLabel = %s N(coincidences) = %5i\n",ModuleLabel,ncoin);

  const mu2e::CrvCoincidence* coin;

  //  int banner_printed = 0;
  for (int i=0; i<ncoin; i++) {
    coin = &coinColl->at(i);
    //    if (banner_printed == 0) {
    //    printCrvCoincidence(coin, "banner");
    // banner_printed = 1;
    //  }

    printCrvCoincidence(coin,"banner+data");
  }
 
}


//-----------------------------------------------------------------------------
void TAnaDump::printCrvCoincidenceCluster(const mu2e::CrvCoincidenceCluster* CCl, 
					  const char* Opt                       ) {
  TString opt = Opt;

  if ((opt == "") || (opt.Index("banner") >= 0)) {
    printf("--------------------------------------------------------------------------------------\n");
    printf("CC Address         Sect   Np   NPe     Tstart      Tend        X          Y          Z\n");
    printf("--------------------------------------------------------------------------------------\n");
  }
 
  if ((opt == "") || (opt.Index("data") >= 0)) {

    const std::vector<art::Ptr<mu2e::CrvRecoPulse>>* list_of_pulses = &CCl->GetCrvRecoPulses();

    int sector      = CCl->GetCrvSectorType();
    int np          = list_of_pulses->size();

    float x         = CCl->GetAvgHitPos().x();
    float y         = CCl->GetAvgHitPos().y();
    float z         = CCl->GetAvgHitPos().z();
    float t1        = CCl->GetStartTime();
    float t2        = CCl->GetEndTime();
    int   npe       = CCl->GetPEs();

    printf("%-16p %5i %5i %5i %10.3f %10.3f %10.3f %10.3f %10.3f\n",static_cast<const void*>(CCl),sector,np,npe,t1,t2,x,y,z);

    const mu2e::CrvRecoPulse* pulse(NULL);
    const mu2e::CrvRecoPulse* otherpulse1(NULL);
    printCrvRecoPulse(pulse, "banner");
    for (int i=0; i<np; i++) {
      pulse = list_of_pulses->at(i).get();
      printCrvRecoPulse(pulse, "data");
    }


    std::map<int, float>   side02, side13;
    std::vector<int>       bars;
    std::vector<int>       sectors;
    float vinvinbar        = 6.5;  // in CRV bars, light travels 6.5ns/m, so this is v inverse in bar

    TCrvNumerology* crvn = TCrvNumerology::Instance();

    for (int i=0; i<np; i++) {
      pulse = list_of_pulses->at(i).get();
      float t0         = pulse->GetLEtime();
      int bar0         = pulse->GetScintillatorBarIndex().asInt();
      int sipm0        = pulse->GetSiPMNumber();

      int   imm, ill, ibb, this_sector;         //module, layer, and bar integers required for GetBarInfo function
      int   sector = crvn->GetBarInfo(bar0,this_sector,imm,ill,ibb);

      if (std::find(bars.begin(), bars.end(), bar0) ==  bars.end()){
	bars.push_back(bar0);
	sectors.push_back(sector); 
      }
      bool found = false;
      for (int j=i+1; j<np; j++) {
	otherpulse1 = list_of_pulses->at(j).get();
	float t1         = otherpulse1->GetLEtime();
	int bar1         = otherpulse1->GetScintillatorBarIndex().asInt();
	int sipm1        = otherpulse1->GetSiPMNumber();
	if (bar0 == bar1) {
	  if ((sipm0%2 == 0) && (sipm1%2 == 0)) {
	    float tavg = (t0 + t1)/2.;
	    side02[bar0] = tavg;
	    found = true;
	    break;
	  }
	  if ((sipm0%2 == 1) && (sipm1%2 == 1)) {
	    float tavg = (t0 + t1)/2.;
	    side13[bar0] = tavg;
	    found = true;
	    break;
	  }
	}
      }//end second loop
      if (found == false) {
	if (sipm0%2 == 1) {
	  side13[bar0] = t0;
	}
	else {
	  side02[bar0] = t0;
	}
      }
    }

    // now print everything
    printf("---------------------------------------------------------------------\n");
    printf("BarIndex       bar_length       Tcorrected(MD)     Xcorrected(MD)    \n");
    printf("---------------------------------------------------------------------\n");
    float totaltimeavg(0),totalxavg(0);
    float nbars = 0;
    int   twoendbars   = 0;
    for (size_t i=0; i<bars.size(); i++) {
	int bar    = bars.at(i);
	int sector = sectors.at(i);
//-----------------------------------------------------------------------------
// only do calculation if you have measured times on both ends for a given bar
//-----------------------------------------------------------------------------
	float bar_length = crvn->BarLength(sector);   // get bar length from array in CrvStubPar_t
	float tcorrected;                                // will be in ns
	float xcorrected;	                         // will be in m

	// printf(" bar, sector, len:  %5i %2i %10.3f\n",bar,sector,bar_length);

	if ((side02.find(bar) != side02.end()) and (side13.find(bar) != side13.end())) {
     
	  tcorrected = .5*(side02[bar] +  side13[bar]  - (bar_length*vinvinbar));
	  xcorrected = .5*(bar_length  - ((side13[bar] - side02[bar])/vinvinbar ));
	  twoendbars += 1;
	  // printf(" case1: time02, time13: %10.3f %10.3f tcorr, xcorr: %10.3f %10.3f 2end_bars: %3i\n",
	  // 	 time02[bar],time13[bar],tcorrected,xcorrected,twoendbars);
	}
	else if (side02.find(bar) != side02.end()) {
//-----------------------------------------------------------------------------
// for simplicity, assume that the hit is in the middle of the bar 
// a more likely assumption would be need to assign the coordinate corresponding to 
// the end on which we have the signal, but at this point I just don't now which 
// end is which, this is to be added
//-----------------------------------------------------------------------------
	  tcorrected = side02[bar];
	  xcorrected = bar_length/2;
	  // printf(" case2: time02: %10.3f tcorr, xcorr: %10.3f %10.3f \n",
	  // 	 time02[bar],tcorrected,xcorrected);
	}
	else {
	  tcorrected = side13[bar];
	  xcorrected = bar_length/2;
	  // printf(" case3: time13: %10.3f tcorr, xcorr: %10.3f %10.3f \n",
	  // 	 time13[bar],tcorrected,xcorrected);
	}
      
	totaltimeavg += tcorrected;
	totalxavg    += xcorrected;
	nbars        += 1;

	printf("%8i %14.2f %16.2f %14.3f\n",bar,bar_length, tcorrected,xcorrected);

	// printf("totaltimeavg, totalxavg, nbars: %10.3f %10.3f %2i\n",totaltimeavg, totalxavg,nbars);
    }
    if (nbars > 0) {
      totaltimeavg /= nbars;
      totalxavg    /= nbars;
    }
    if (twoendbars == 0) {
      totalxavg = -10.;
    }

    //   for (size_t i=0; i<bars.size(); i++) {
    //   int bar = bars.at(i);
    //   if ((side02.find(bar) == side02.end()) || (side13.find(bar) == side13.end())) {
    // 	continue;
    //   }
    //   // now do the calculations using the above side time averages
    //   float topbarlength = 6.; // length of all top CRV bars is 6m
    //   float vinvinbar = 6.5; // in CRV bars, light travels 6.5ns/m, so this is v inverse in bar
    //   float tcorrected; // will be in ns
    //   float xcorrected; // will be in m
    //   nbars += 1;
      
    //   tcorrected = .5*(side02[bar] + side13[bar] - (topbarlength*vinvinbar));
    //   xcorrected = .5*(topbarlength - ((side13[bar] - side02[bar])/vinvinbar));
      
    //   totaltimeavg += tcorrected;
      
    //   printf("%8i %9f %9f\n",bar,tcorrected,xcorrected);
    // }
    // totaltimeavg /= nbars;
    printf("total coincidence corrected time average: %8f\n", totaltimeavg);
  }
  
  if (opt.Index("hits") >= 0) {

  }
}


//-----------------------------------------------------------------------------
void TAnaDump::printCrvCoincidenceClusterCollection(const char* ModuleLabel, 
						    const char* ProductName,
						    const char* ProcessName) {

  art::Handle<mu2e::CrvCoincidenceClusterCollection> handle;
  const mu2e::CrvCoincidenceClusterCollection*       ccColl;

  if (ProductName[0] != 0) {
    art::Selector  selector(art::ProductInstanceNameSelector(ProductName) &&
			    art::ProcessNameSelector(ProcessName)         && 
			    art::ModuleLabelSelector(ModuleLabel)            );
    fEvent->get(selector,handle);
  }
  else {
    art::Selector  selector(art::ProcessNameSelector(ProcessName)         && 
			    art::ModuleLabelSelector(ModuleLabel)            );
    fEvent->get(selector,handle);
  }
//-----------------------------------------------------------------------------
// make sure collection exists
//-----------------------------------------------------------------------------
  if (! handle.isValid()) {
    printf("TAnaDump::printCrvCoincidenceClusterCollection: no CrvCoincidenceClusterCollection ");
    printf("for module %s and ProductName=%s found, BAIL OUT\n",
	   ModuleLabel,ProductName);
    return;
  }

  ccColl = handle.product();

  int ncc = ccColl->size();

  printf(">>>> ModuleLabel = %s N(coincidence clusters) = %5i\n",ModuleLabel,ncc);

  const mu2e::CrvCoincidenceCluster* cc;

  //  int banner_printed = 0;
  for (int i=0; i<ncc; i++) {
    cc = &ccColl->at(i);
    //    if (banner_printed == 0) {
    //      printCrvCoincidenceCluster(cc, "banner");
    //      banner_printed = 1;
    //    }

    printCrvCoincidenceCluster(cc,"banner+data");
  }
 
}


//-----------------------------------------------------------------------------
void TAnaDump::printCrvRecoPulse(const mu2e::CrvRecoPulse* Pulse, 
				 const char* Opt                ) {

  TString opt = Opt;

  if ((opt == "") || (opt.Index("banner") >= 0)) {
    printf("-------------------------------------------------------------------------------------------------------\n");
    printf("Pulse Addr         NPE   HPE    Time    Height    Width     Chi2    LeTime   Bar   Sipm  NInd   Indices\n");
    printf("-------------------------------------------------------------------------------------------------------\n");
  }
 
  if ((opt == "") || (opt.Index("data") >= 0)) {

    int npes        = Pulse->GetPEs();
    int npes_height = Pulse->GetPEsPulseHeight();
    int nind        = Pulse->GetWaveformIndices().size();
    float time      = Pulse->GetPulseTime();
    float height    = Pulse->GetPulseHeight();
    float width     = Pulse->GetPulseBeta(); // was GetPulseWidth();
    float chi2      = Pulse->GetPulseFitChi2();
    float le_time   = Pulse->GetLEtime();

    int bar         = Pulse->GetScintillatorBarIndex().asInt();
    int sipm_number = Pulse->GetSiPMNumber();

    printf("%-16p %5i %5i %8.3f %8.3f %8.3f %10.3f %8.3f %5i %5i %5i",
     	   static_cast<const void*>(Pulse),
	   npes,
	   npes_height,
	   time,
	   height,
	   width,
	   chi2,
	   le_time,
	   bar,
	   sipm_number,
	   nind);

    for (int i=0; i<nind; i++) {
      int ind =  Pulse->GetWaveformIndices().at(i);
      printf("%5i",ind);
    }
    printf("\n");
  }
  
  if (opt.Index("hits") >= 0) {

  }
}

//----------------------------------------------------------------------------
void TAnaDump::printCrvDigi(const mu2e::CrvDigi* Digi, 
			    const char* Opt                ) {

  TString opt = Opt;

  if ((opt == "") || (opt.Index("banner") >= 0)) {
    printf("-------------------------------------------------------------------------------------------------------\n");
    printf(" ADC0   ADC1   ADC2   ADC3   ADC4   ADC5   ADC6   ADC7   StartTDC  BarIndex  SiPM#\n");
    printf("-------------------------------------------------------------------------------------------------------\n");
  }
 
  if ((opt == "") || (opt.Index("data") >= 0)) {

    int adc0        = Digi->GetADCs().at(0);
    int adc1        = Digi->GetADCs().at(1);
    int adc2        = Digi->GetADCs().at(2);
    int adc3        = Digi->GetADCs().at(3);
    int adc4        = Digi->GetADCs().at(4);
    int adc5        = Digi->GetADCs().at(5);
    int adc6        = Digi->GetADCs().at(6);
    int adc7        = Digi->GetADCs().at(7);
    float tdc       = Digi->GetStartTDC() * 12.55; //now in ns with digitization factor to convert

    int bar         = Digi->GetScintillatorBarIndex().asInt();
    int sipm_number = Digi->GetSiPMNumber();

    printf("%5i %6i %6i %6i %6i %6i %6i %6i %9.2f %7i %8i",
	   adc0,
	   adc1,
	   adc2,
	   adc3,
	   adc4,
	   adc5,
	   adc6,
	   adc7,
	   tdc,
	   bar,
	   sipm_number);

    printf("\n");
  }
  
  if (opt.Index("hits") >= 0) {

  }
}

//-----------------------------------------------------------------------------
// Print reconstructed CRV digi
//-----------------------------------------------------------------------------
void TAnaDump::printCrvDigiCollection(const char* ModuleLabel, 
					   const char* ProductName,
					   const char* ProcessName) {

  art::Handle<mu2e::CrvDigiCollection> handle;
  const mu2e::CrvDigiCollection*       crpColl;

  if (ProductName[0] != 0) {
    art::Selector  selector(art::ProductInstanceNameSelector(ProductName) &&
			    art::ProcessNameSelector(ProcessName)         && 
			    art::ModuleLabelSelector(ModuleLabel)            );
    fEvent->get(selector,handle);

    // art::Selector  selectorMC(art::ProductInstanceNameSelector(ProductName) &&
    // 			      art::ProcessNameSelector(ProcessName)         && 
    // 			      art::ModuleLabelSelector(MCModuleLabel)            );
   
    // fEvent->get(selectorMC,caloHitTruthHandle);
  }
  else {
    art::Selector  selector(art::ProcessNameSelector(ProcessName)         && 
			    art::ModuleLabelSelector(ModuleLabel)            );
    fEvent->get(selector,handle);
    // art::Selector  selectorMC(art::ProcessNameSelector(ProcessName)         && 
    // 			      art::ModuleLabelSelector(MCModuleLabel)            );
    // fEvent->get(selectorMC,caloHitTruthHandle);
  }
//-----------------------------------------------------------------------------
// make sure collection exists
//-----------------------------------------------------------------------------
  if (! handle.isValid()) {
    printf("TAnaDump::printCrvDigiCollection: no CrvDigiCollection ");
    printf("for module %s and ProductName=%s found, BAIL OUT\n",
	   ModuleLabel,ProductName);
    return;
  }

  crpColl = handle.product();

  int npulses = crpColl->size();

  printf(">>>> ModuleLabel = %s N(reco pulses) = %5i\n",ModuleLabel,npulses);

  //  if (caloHitTruthHandle.isValid()) caloHitTruth = caloHitTruthHandle.product();

  const mu2e::CrvDigi* pulse;


  int banner_printed = 0;
  for (int i=0; i<npulses; i++) {
    pulse = &crpColl->at(i);
    if (banner_printed == 0) {
      printCrvDigi(pulse, "banner");
      banner_printed = 1;
    }

    printCrvDigi(pulse,"data");
  }
 
}


//-----------------------------------------------------------------------------
// Print reconstructed CRV pulses (processed waveforms)
//-----------------------------------------------------------------------------
void TAnaDump::printCrvRecoPulseCollection(const char* ModuleLabel, 
					   const char* ProductName,
					   const char* ProcessName) {

  art::Handle<mu2e::CrvRecoPulseCollection> handle;
  const mu2e::CrvRecoPulseCollection*       crpColl;

  if (ProductName[0] != 0) {
    art::Selector  selector(art::ProductInstanceNameSelector(ProductName) &&
			    art::ProcessNameSelector(ProcessName)         && 
			    art::ModuleLabelSelector(ModuleLabel)            );
    fEvent->get(selector,handle);

    // art::Selector  selectorMC(art::ProductInstanceNameSelector(ProductName) &&
    // 			      art::ProcessNameSelector(ProcessName)         && 
    // 			      art::ModuleLabelSelector(MCModuleLabel)            );
   
    // fEvent->get(selectorMC,caloHitTruthHandle);
  }
  else {
    art::Selector  selector(art::ProcessNameSelector(ProcessName)         && 
			    art::ModuleLabelSelector(ModuleLabel)            );
    fEvent->get(selector,handle);
    // art::Selector  selectorMC(art::ProcessNameSelector(ProcessName)         && 
    // 			      art::ModuleLabelSelector(MCModuleLabel)            );
    // fEvent->get(selectorMC,caloHitTruthHandle);
  }
//-----------------------------------------------------------------------------
// make sure collection exists
//-----------------------------------------------------------------------------
  if (! handle.isValid()) {
    printf("TAnaDump::printCrvRecoPulseCollection: no CrvRecoPulseCollection ");
    printf("for module %s and ProductName=%s found, BAIL OUT\n",
	   ModuleLabel,ProductName);
    return;
  }

  crpColl = handle.product();

  int npulses = crpColl->size();

  printf(">>>> ModuleLabel = %s N(reco pulses) = %5i\n",ModuleLabel,npulses);

  //  if (caloHitTruthHandle.isValid()) caloHitTruth = caloHitTruthHandle.product();

  const mu2e::CrvRecoPulse* pulse;


  int banner_printed = 0;
  for (int i=0; i<npulses; i++) {
    pulse = &crpColl->at(i);
    if (banner_printed == 0) {
      printCrvRecoPulse(pulse, "banner");
      banner_printed = 1;
    }

    printCrvRecoPulse(pulse,"data");
  }
 
}


//-----------------------------------------------------------------------------
void TAnaDump::printEventHeader() {

  printf(" Run / Subrun / Event : %10i / %10i / %10i\n",
	 fEvent->run(),
	 fEvent->subRun(),
	 fEvent->event());
}

// //-----------------------------------------------------------------------------
// void TAnaDump::printTrkCaloHit(const KalRep* Krep, mu2e::TrkCaloHit* CaloHit){
//   double    len  = CaloHit->fltLen();
//   HepPoint  plen = Krep->position(len);
  
//   printf("%3i %5i 0x%08x %1i %9.3f %8.3f %8.3f %9.3f %8.3f %7.3f",
// 	 -1,//++i,
// 	 0, //straw->index().asInt(), 
// 	 CaloHit->hitFlag(),
// 	 //	     hit->isUsable(),
// 	 CaloHit->isActive(),
// 	 len,
// 	 //	     hit->hitRms(),
// 	 plen.x(),plen.y(),plen.z(),
// 	 CaloHit->time(), -1.//sh->dt()
// 	 );

//   printf(" %2i %2i %2i %2i",
// 	 -1,//straw->id().getPlane(),
// 	 -1,//straw->id().getPanel(),
// 	 -1,//straw->id().getLayer(),
// 	 -1//straw->id().getStraw()
// 	 );

//   printf(" %8.3f",CaloHit->hitT0().t0());
  
//   double res, sigres;
//   CaloHit->resid(res, sigres, true);

//   CLHEP::Hep3Vector  pos;
//   CaloHit->hitPosition(pos);

//   printf("%8.3f %8.3f %9.3f %7.3f %7.3f",
// 	 pos.x(),
// 	 pos.y(),
// 	 pos.z(),
// 	 res,
// 	 sigres
// 	 );
      
//   printf("   %6.3f", -1.);//CaloHit->driftRadius());
  
	  

//   printf("  %7.3f",-1.);

//   double exterr = CaloHit->temperature();//*CaloHit->driftVelocity();

//   printf(" %6.3f %6.3f %6.3f %6.3f %6.3f",		 
// 	 -1.,//CaloHit->totalErr(),
// 	 CaloHit->hitErr(),
// 	 CaloHit->hitT0().t0Err(),
// 	 -1.,//,CaloHit->penaltyErr(),
// 	 exterr
// 	 );
//   //-----------------------------------------------------------------------------
//   // test: calculated residual in fTmp[0]
//   //-----------------------------------------------------------------------------
//   //       Test_000(Krep,hit);
//   //       printf(" %7.3f",fTmp[0]);

//   printf("\n");

// }

// //-----------------------------------------------------------------------------
// // ""       : banner+track parameters (default)
// // "banner" : banner only
// // "data"   : track parameters only
// // "hits"   : hits
// //-----------------------------------------------------------------------------
// void TAnaDump::printKalRep(const KalRep* Krep, const char* Opt, const char* Prefix) {

//   //  TString opt = Opt;

//   _printUtils->printTrack(fEvent,Krep,Opt,Prefix);
// }

// //-----------------------------------------------------------------------------
// void TAnaDump::printKalRepCollection(const char* KalRepCollTag     , 
// 				     int         hitOpt            ,
// 				     const char* StrawDigiMCCollTag) {

//   art::InputTag                          krepCollTag(KalRepCollTag);
//   art::Handle<mu2e::KalRepPtrCollection> krepsHandle; 
// //-----------------------------------------------------------------------------
// // make sure collection exists
// //-----------------------------------------------------------------------------
//   fEvent->getByLabel(krepCollTag,krepsHandle);
//   if (! krepsHandle.isValid()) {
//     printf("TAnaDump::printKalRepCollection: no KalRepPtrCollection tag=%s, BAIL OUT\n", KalRepCollTag);
//     printf(" available ones are:\n");
//     print_kalrep_colls();
//     return;
//   }

//   art::InputTag sdmc_tag = StrawDigiMCCollTag;
//   if (sdmc_tag == "") sdmc_tag = fSdmcCollTag;

//   art::Handle<mu2e::StrawDigiMCCollection> sdmccH;
//   fEvent->getByLabel<mu2e::StrawDigiMCCollection>(sdmc_tag,sdmccH);

//   if (sdmccH.isValid()) _mcdigis = sdmccH.product();
//   else                  _mcdigis = nullptr;

//   if (_mcdigis == nullptr) {
//     printf(">>> ERROR in TAnaDump::printKalRepCollection: failed to locate StepPointMCCollection:: by %s\n",
//            sdmc_tag.encode().data());
//   }

//   int ntrk = krepsHandle->size();

//   const KalRep *trk;

//   int banner_printed = 0;
//   for (int i=0; i<ntrk; i++) {
//     art::Ptr<KalRep> kptr = krepsHandle->at(i);
//     //    fEvent->get(kptr.id(), krepsHandle);
//     fhicl::ParameterSet const& pset = krepsHandle.provenance()->parameterSet();
//     string module_type = pset.get<string>("module_type");
 
//     trk = kptr.get();
//     if (banner_printed == 0) {
//       printKalRep(trk,"banner",module_type.data());
//       banner_printed = 1;
//     }
//     printKalRep(trk,"data",module_type.data());
//     if (hitOpt > 0) printKalRep(trk,"hits");
//   }
// }


//-----------------------------------------------------------------------------
void TAnaDump::printGenParticle(const mu2e::GenParticle* P, const char* Opt) {

  TString opt = Opt;
  
  if ((opt == "") || (opt == "banner")) {
    printf("------------------------------------------------------------------------------------\n");
    printf("Index                 generator     PDG      Time      Momentum       Pt       CosTh\n");
    printf("------------------------------------------------------------------------------------\n");
  }
  
  if ((opt == "") || (opt == "data")) {
    int    gen_code   = P->generatorId().id();
    string gen_name   = P->generatorId().name();
    int    pdg_code   = P->pdgId();
    double time       = P->time();
    
    double mom   = P->momentum().vect().mag();
    double pt    = P->momentum().vect().perp();
    double costh = P->momentum().vect().cosTheta();
    
    printf("%5i %2i:%-26s %3i %10.3f %10.3f %10.3f %10.3f\n",
	   -1,gen_code,gen_name.data(),pdg_code,time,mom,pt,costh);
  }
}

//-----------------------------------------------------------------------------
// there could be multiple collections in the event
//-----------------------------------------------------------------------------
void TAnaDump::printGenParticleCollections() {
  
  vector<art::Handle<mu2e::GenParticleCollection>> list_of_gp;

  const mu2e::GenParticleCollection*   coll(0);
  const mu2e::GenParticle*        genp(0);

  const art::Provenance* prov;

  //  art::Selector  selector(art::ProductInstanceNameSelector("mu2e::GenParticleCollection"));
  art::Selector  selector(art::ProductInstanceNameSelector(""));

  list_of_gp = fEvent->getMany<mu2e::GenParticleCollection>(selector);

  const art::Handle<mu2e::GenParticleCollection>* handle;

  int banner_printed;
  for (vector<art::Handle<mu2e::GenParticleCollection>> ::const_iterator it = list_of_gp.begin();
       it != list_of_gp.end(); it++) {
    handle = it.operator -> ();

    if (handle->isValid()) {
      coll = handle->product();
      prov = handle->provenance();

      printf("moduleLabel = %-20s, producedClassname = %-30s, productInstanceName = %-20s\n",
	     prov->moduleLabel().data(),
	     prov->producedClassName().data(),
	     prov->productInstanceName().data());

      banner_printed = 0;
      for (vector<mu2e::GenParticle>::const_iterator ip = coll->begin();
	   ip != coll->end(); ip++) {
	genp = ip.operator -> ();
	if (banner_printed == 0) {
	  printGenParticle(genp,"banner");
	  banner_printed = 1;
	}
	printGenParticle(genp,"data");
      }

      
    }
    else {
      printf(">>> ERROR in TAnaDump::printStepPointMCCollection: failed to locate collection");
      printf(". BAIL OUT. \n");
      return;
    }
  }
}


// //-----------------------------------------------------------------------------
//   void TAnaDump::printCaloHit(const CaloHit* Hit, const char* Opt) {
//     //    int row, col;
//     TString opt = Opt;

//     if ((opt == "") || (opt == "banner")) {
//       printf("--------------------------------------\n");
//       printf("RID      Time   Energy                \n");
//       printf("--------------------------------------\n");
//     }
    
//     if ((opt == "") || (opt == "data")) {
//       printf("%7i  %10.3f %10.3f \n",
// 	     Hit->id(),
// 	     Hit->time(),
// 	     Hit->energyDep()); 
//     }
//   }


//-----------------------------------------------------------------------------
void TAnaDump::printCaloHits(const char* ModuleLabel, 
			     const char* ProductName, 
			     const char* ProcessName) {

  printf(">>>> ModuleLabel = %s\n",ModuleLabel);

  //data about hits in the calorimeter crystals

  art::Selector  selector(art::ProductInstanceNameSelector(ProductName) &&
			  art::ProcessNameSelector(ProcessName)         && 
			  art::ModuleLabelSelector(ModuleLabel)            );

  art::Handle<mu2e::CaloHitCollection> caloHitsHandle;

  fEvent->get(selector,caloHitsHandle);

  const mu2e::CaloHitCollection* caloHits;

  caloHits = caloHitsHandle.operator ->();

  int nhits = caloHits->size();

  const mu2e::CaloHit* hit;

  printf("--------------------------------------\n");
  printf("RID      Time   Energy                \n");
  printf("--------------------------------------\n");

  for (int ic=0; ic<nhits; ic++) {
    hit  = &caloHits->at(ic);
    printf("%7i  %10.3f %10.3f \n",
	   hit->crystalID(),
	   hit->time(),
	   hit->energyDep()); 
  }
}


//-----------------------------------------------------------------------------
void TAnaDump::printCalorimeter() {
  const mu2e::DiskCalorimeter* cal;
  const mu2e::Disk* disk;
  
  art::ServiceHandle<mu2e::GeometryService> geom;
    
  if (geom->hasElement<mu2e::DiskCalorimeter>() ) {
    mu2e::GeomHandle<mu2e::DiskCalorimeter> dc;
    cal = dc.operator->();
  }
  else {
    printf(">>> ERROR: disk calorimeter not found.\n");
    return;
  }

  int nd = cal->nDisks();
  printf(" ndisks = %i\n", nd);
  printf(" crystal size  : %10.3f\n", cal->caloInfo().getDouble("crystalXYLength"));
  printf(" crystal length: %10.3f\n", cal->caloInfo().getDouble("crystalZLength"));

  for (int i=0; i<nd; i++) {
    disk = &cal->disk(i);
    printf(" ---- disk # %i\n",i);
    printf(" Rin  : %10.3f  Rout : %10.3f\n", disk->geomInfo().innerEnvelopeR(), disk->geomInfo().outerEnvelopeR());
    printf(" X : %12.3f Y : %12.3f Z : %12.3f\n",
	   disk->geomInfo().origin().x(),
	   disk->geomInfo().origin().y(),
	   disk->geomInfo().origin().z());
    // printf(" Xsize : %10.3f Ysize : %10.3f Zsize : %10.3f\n", 
    // 	   disk->size().x(),
    // 	   disk->size().y(),
    // 	   disk->size().z()
    // 	   );
  }
}


//-----------------------------------------------------------------------------
void TAnaDump::printCaloCrystalHits(const char* ModuleLabel, 
				    const char* ProductName,
				    const char* ProcessName) {

  art::Selector  selector(art::ProductInstanceNameSelector(ProductName) &&
			  art::ProcessNameSelector(ProcessName)         && 
			  art::ModuleLabelSelector(ModuleLabel)            );

  art::Handle<mu2e::CaloHitCollection> caloCrystalHitsHandle;

  fEvent->get(selector,caloCrystalHitsHandle);

  const mu2e::CaloHitCollection* caloCrystalHits;

  caloCrystalHits = caloCrystalHitsHandle.product();// operator->();

  int nhits = caloCrystalHits->size();

  const mu2e::CaloHit* hit;

  printf("----------------------------------------------------------------\n");
  printf("CrystalID      Time   Energy    EnergyTot  NSiPMs               \n");
  printf("----------------------------------------------------------------\n");

  for (int ic=0; ic<nhits; ic++) {
    hit  = &caloCrystalHits->at(ic);

    printf("%7i  %10.3f %10.3f %10.3f %5i\n",
	   hit->crystalID(),
	   hit->time(),
	   hit->energyDep(),
	   hit->energyDepTot(),
	   hit->nSiPMs());
  }
}

//-----------------------------------------------------------------------------
void TAnaDump::printCaloDigiCollection(const char* ModuleLabel, 
				       const char* ProductName,
				       const char* ProcessName) {

  art::Selector  selector(art::ProductInstanceNameSelector(ProductName) &&
			  art::ProcessNameSelector(ProcessName)         && 
			  art::ModuleLabelSelector(ModuleLabel)            );

  art::Handle<mu2e::CaloDigiCollection> calodigisHandle;

  fEvent->get(selector,calodigisHandle);

  const mu2e::CaloDigiCollection* calodigis;

  calodigis = calodigisHandle.operator->();

  int nhits = calodigis->size();

  const mu2e::CaloDigi* hit;

  printf("----------------------------------------------------------------\n");
  printf("ReadoutID      Time      NSamples               \n");
  printf("----------------------------------------------------------------\n");

  for (int ic=0; ic<nhits; ic++) {
    hit  = &calodigis->at(ic);
    int pulse_size =  hit->waveform().size();

    printf("%7i  %5i %5i\n",
	   hit->SiPMID(),
	   hit->t0(),
	   pulse_size);
  }
}


//-----------------------------------------------------------------------------
void TAnaDump::printCaloRecoDigiCollection(const char* ModuleLabel, 
				       const char* ProductName,
				       const char* ProcessName) {

  art::Selector  selector(art::ProductInstanceNameSelector(ProductName) &&
			  art::ProcessNameSelector(ProcessName)         && 
			  art::ModuleLabelSelector(ModuleLabel)            );

  art::Handle<mu2e::CaloRecoDigiCollection> recocalodigisHandle;

  fEvent->get(selector,recocalodigisHandle);

  const mu2e::CaloRecoDigiCollection* recocalodigis;

  recocalodigis = recocalodigisHandle.operator->();

  int nhits = recocalodigis->size();

  const mu2e::CaloRecoDigi* hit;

  printf("-----------------------------------------------------------------------------------\n");
  printf("ReadoutID      Time      Time-Chi2     Energy     Amplitude      PSD               \n");
  printf("-----------------------------------------------------------------------------------\n");

  for (int ic=0; ic<nhits; ic++) {
    hit  = &recocalodigis->at(ic);

    printf("%7i  %10.3f   %10.3f   %10.3f   %10.3f   %10.3f\n",
	   hit->SiPMID(),
	   hit->time(),
	   hit->chi2(), 
	   hit->energyDep(),
	   -1., //hit->amplitude(),
	   -1.);//hit->psd());
  }
}

////////////////////////////////////////////////////////////////////////////////

// void TAnaDump::printTrkToCaloExtrapol(const mu2e::TrkToCaloExtrapol* trkToCalo,
// 				      const char* Opt) {
//  TString opt = Opt;

//   if ((opt == "") || (opt == "banner")) {
//     printf("-------------------------------------------------------------------------------------------------------\n");
//     printf("sectionId      Time     ExtPath     Ds       FitCon      t0          X           Y        Z          Mom  \n");
//     printf("-------------------------------------------------------------------------------------------------------\n");
//   }
  
//   if ((opt == "") || (opt.Index("data") >= 0)) {

//     double ds = trkToCalo->pathLengthExit()-trkToCalo->pathLengthEntrance();
  
//     printf("%6i %10.3f %10.3f %8.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f \n",
// 	   trkToCalo->diskId(),
// 	   trkToCalo->time(),
// 	   trkToCalo->pathLengthEntrance(),
// 	   ds,
// 	   trkToCalo->fitConsistency(),
// 	   trkToCalo->t0(),
// 	   trkToCalo->entrancePosition().x(),
// 	   trkToCalo->entrancePosition().y(),
// 	   trkToCalo->entrancePosition().z(),
// 	   trkToCalo->momentum().mag() );
//   }
  
// }

////////////////////////////////////////////////////////////////////////////////

// void TAnaDump::printTrkToCaloExtrapolCollection(const char* ModuleLabel, 
// 						const char* ProductName,
// 						const char* ProcessName) {

//   printf(">>>> ModuleLabel = %s\n",ModuleLabel);

//   //data about hits in the calorimeter crystals

//   art::Handle<mu2e::TrkToCaloExtrapolCollection> trkToCaloHandle;
//   const mu2e::TrkToCaloExtrapolCollection* trkToCalo;

//   if (ProductName[0] != 0) {
//     art::Selector  selector(art::ProductInstanceNameSelector(ProductName) &&
// 			    art::ProcessNameSelector(ProcessName)         && 
// 			    art::ModuleLabelSelector(ModuleLabel)            );
//     fEvent->get(selector, trkToCaloHandle);
//   }
//   else {
//     art::Selector  selector(art::ProcessNameSelector(ProcessName)         && 
// 			    art::ModuleLabelSelector(ModuleLabel)            );
//     fEvent->get(selector, trkToCaloHandle);
//   }

//   trkToCalo = trkToCaloHandle.operator ->();

//   int nhits = trkToCalo->size();

//   const mu2e::TrkToCaloExtrapol* hit;
  
//   int banner_printed = 0;
//   for (int i=0; i<nhits; i++) {
//     hit = &trkToCalo->at(i);
//     if (banner_printed == 0) {
//       printTrkToCaloExtrapol(hit, "banner");
//       banner_printed = 1;
//     }
//     printTrkToCaloExtrapol(hit,"data");
//   }
  
// }



//-----------------------------------------------------------------------------
void TAnaDump::printStrawHit(const mu2e::StrawHit* Hit, const mu2e::StrawGasStep* Step, const char* Opt, int IHit, int Flags) {
  TString opt = Opt;
  opt.ToLower();

  if ((opt == "") || (opt.Index("banner") >= 0)) {
    printf("\n");
    printf("#-------------------------------------------------------------------");
    printf("------------------------------------------------------------\n");
    printf("#   I   SID    Flags  Pln:Pnl:Lay:Str   Time    TOT     dt     eDep ");
    printf("           PDG       PDG(M)   Generator      SimpID      p  \n");
    printf("#-------------------------------------------------------------------");
    printf("------------------------------------------------------------\n");
  }

  if (opt == "banner") return;

  mu2e::GeomHandle<mu2e::Tracker> ttH;
  const mu2e::Tracker* tracker = ttH.get();

  const mu2e::Straw* straw = &tracker->getStraw(Hit->strawId());

  const mu2e::SimParticle* sim(0);
    
  int      pdg_id(-1), mother_pdg_id(-1), generator_id(-1), simp_id(-1);
  double   mc_mom(-1.);

  mu2e::GenId gen_id;

  if (Step) {
    art::Ptr<mu2e::SimParticle> const& simptr = Step->simParticle(); 
    art::Ptr<mu2e::SimParticle> mother = simptr;
    
    while(mother->hasParent()) mother = mother->parent();
      
    sim = mother.operator ->();

    pdg_id        = simptr->pdgId();
    mother_pdg_id = sim->pdgId();

    if (simptr->fromGenerator()) generator_id = simptr->genParticle()->generatorId().id();
    else                         generator_id = -1;

    simp_id       = simptr->id().asInt();
    mc_mom        = Step->momvec().mag();
  }
    
  if ((opt == "") || (opt.Index("data") >= 0)) {
    if (IHit  >= 0) printf("%5i " ,IHit);
    else            printf("    ");
    
    printf("%5i",Hit->strawId().asUint16());

    if (Flags >= 0) printf(" %08x",Flags);
    else            printf("        ");
    printf(" %3i %3i %3i %3i %8.2f %6.2f %6.2f %9.6f   %10i   %10i  %10i  %10i %8.3f\n",
	   straw->id().getPlane(),
	   straw->id().getPanel(),
	   straw->id().getLayer(),
	   straw->id().getStraw(),
	   Hit->time(),
	   Hit->TOT(),
	   Hit->dt(),
	   Hit->energyDep(),
	   pdg_id,
	   mother_pdg_id,
	   generator_id,
	   simp_id,
	   mc_mom);
  }
}

//-----------------------------------------------------------------------------
void TAnaDump::printStrawHitCollection(const char* StrawHitCollTag   , 
				       const char* StrawDigiMCCollTag, 
				       double TMin, double TMax) {

  const char* oname = "TAnaDump::printStrawHitCollection";

  art::Handle<mu2e::StrawHitCollection> shcH;
  const mu2e::StrawHitCollection*       shc;

  art::Handle<mu2e::ComboHitCollection> chcH;
  const mu2e::ComboHitCollection*       chc;
//-----------------------------------------------------------------------------
// get straw hits
//-----------------------------------------------------------------------------
  bool ok = fEvent->getByLabel(StrawHitCollTag, shcH);
  if (ok) shc = shcH.product();
  else {
    printf(">>> ERROR in %s: Straw Hit Collection by \"%s\" doesn't exist. Bail Out.\n",
	   oname,StrawHitCollTag);
    return;
  }

  ok = fEvent->getByLabel(StrawHitCollTag, chcH);
  if (ok) chc = chcH.product();
  else {
    printf(">>> ERROR in %s: ComboHitCollection \"%s\" doesn't exist. Bail Out.\n",
	   oname,StrawHitCollTag);
    return;
  }

  art::InputTag sdmcc_tag = StrawDigiMCCollTag;
  if (sdmcc_tag.empty()) sdmcc_tag = fSdmcCollTag;

  art::Handle<mu2e::StrawDigiMCCollection> mcdh;
  fEvent->getByLabel<mu2e::StrawDigiMCCollection>(sdmcc_tag,mcdh);

  if (mcdh.isValid()) _mcdigis = mcdh.product();
  else                _mcdigis = nullptr;

  if (_mcdigis == nullptr) {
    printf(">>> ERROR in %s: failed to locate StrawDigiMCCollection with tag=%s, BAIL OUT.\n",
	   oname,fSdmcCollTag.encode().data());
    return;
  }
 
  int nhits = shc->size();

  int                       flags;
  int                       banner_printed(0);
  for (int i=0; i<nhits; i++) {
    const mu2e::StrawHit* hit = &shc->at(i);
    const mu2e::StrawDigiMC*  sdmc = &_mcdigis->at(i);
    const mu2e::StrawGasStep* step = sdmc->earlyStrawGasStep().get();
					// assuming it doesn't move beyond 32 bits
    flags = *((int*) &chc->at(i).flag());
    if (banner_printed == 0) {
      printStrawHit(hit,step,"banner");
      banner_printed = 1;
    }
    if ((hit->time() >= TMin) && (hit->time() <= TMax)) {
      printStrawHit(hit,step,"data",i,flags);
    }
  }
 
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void TAnaDump::printStrawGasStep(const mu2e::StrawGasStep* Step, const char* Opt, int IStep) {
  
  TString opt = Opt;
  opt.ToLower();

  if ((opt == "") || (opt.Index("banner") >= 0)) {
    printf("----------------------------------------------------------------------------------------------------------------------");
    printf("---------------------------------------------------------------------------------------------\n");
    printf("    I   SID  Plane  Panel   Layer   Straw   Stype     EIon   PathLen    Width");
    printf("       Time        PDG      PDG(M)       GenID       SimID   X0          Y0          Z0        ");
    printf("X1         Y1          Z1           Mom\n");
    printf("----------------------------------------------------------------------------------------------------------------------");
    printf("---------------------------------------------------------------------------------------------\n");
  }

  if (opt == "banner") return;

  mu2e::GeomHandle<mu2e::Tracker> ttH;
  const mu2e::Tracker* tracker = ttH.get();

  const mu2e::Straw* straw = &tracker->getStraw(Step->strawId());

  const mu2e::SimParticle* sim(0);
    
  int      pdg_id(-1), mother_pdg_id(-1), generator_id(-1), simp_id(-1);
  double   mc_mom(-1.);

  mu2e::GenId gen_id;

  art::Ptr<mu2e::SimParticle> const& simptr = Step->simParticle(); 
  art::Ptr<mu2e::SimParticle> mother = simptr;
    
  while(mother->hasParent()) mother = mother->parent();
      
  sim = mother.operator ->();

  pdg_id        = simptr->pdgId();
  mother_pdg_id = sim->pdgId();

  if (simptr->fromGenerator()) generator_id = simptr->genParticle()->generatorId().id();
  else                         generator_id = -1;

  simp_id       = simptr->id().asInt();
  mc_mom        = Step->momvec().mag();
    
  if ((opt == "") || (opt == "data")) {
    if (IStep  >= 0) printf("%5i " ,IStep);
    else             printf("    ");
    
    printf("%5i",Step->strawId().asUint16());

    printf("  %5i  %5i   %5i   %5i   %5i",
	   straw->id().getPlane(),
	   straw->id().getPanel(),
	   straw->id().getLayer(),
	   straw->id().getStraw(),
	   (int) Step->stepType()._stype);

    float stepTime;

    stepTime = Step->time();

    printf(" %8.3f  %8.3f %8.3f  %9.3f %10i  %10i  %10i  %10i",
	   Step->ionizingEdep(),
	   Step->stepLength(),
	   Step->width(),
	   stepTime,
	   pdg_id,
	   mother_pdg_id,
	   generator_id,
	   simp_id);

    printf(" %8.3f   %8.3f   %9.3f  %8.3f   %8.3f   %9.3f   %8.3f\n",
	   Step->startPosition().x(),
	   Step->startPosition().y(),
	   Step->startPosition().z(),
	   Step->endPosition().x(),
	   Step->endPosition().y(),
	   Step->endPosition().z(),
	   mc_mom);
  }
} 

//-----------------------------------------------------------------------------
// FlagBgrHitsCollName = 'StrawHits' or 'ComboHits'. Very unfortunate choice!
//-----------------------------------------------------------------------------
void TAnaDump::printStrawGasStepCollection(const char* CollTag, 
					   double      TMin   , 
					   double      TMax)  {

  //  const char* oname = "TAnaDump::printStrawGasStepCollection";
//-----------------------------------------------------------------------------
// get straw hits
//-----------------------------------------------------------------------------
  art::Handle<mu2e::StrawGasStepCollection> sgscH;
  const mu2e::StrawGasStepCollection* sgsc(nullptr);
  fEvent->getByLabel(CollTag,sgscH);

  if (sgscH.isValid()) sgsc = sgscH.product();
  else {
    printf("ERROR: cant find StrawHitCollection tag=%s, print available, EXIT\n",CollTag);

    // vector<art::Handle<mu2e::StrawGasStepCollection>> vcoll;
    art::Selector  selector(art::ProductInstanceNameSelector(""));
    auto vcoll = fEvent->getMany<mu2e::StrawGasStepCollection>(selector);

    for (auto it = vcoll.begin(); it != vcoll.end(); it++) {
      const art::Handle<mu2e::StrawGasStepCollection>*  handle = it.operator -> ();
      if (handle->isValid()) {
	const art::Provenance* prov = handle->provenance();
	
	printf("moduleLabel: %-20s, productInstanceName: %-20s, processName:= %-30s\n" ,
	       prov->moduleLabel().data(),
	       prov->productInstanceName().data(),
	       prov->processName().data()
	       );
      }
    }
    return;
  }

  int banner_printed(0);

  int nhits = sgsc->size();
  for (int i=0; i<nhits; i++) {
    const mu2e::StrawGasStep* step = &sgsc->at(i);

    if (banner_printed == 0) {
      printStrawGasStep(step, "banner");
      banner_printed = 1;
    }
    if ((step->time() >= TMin) && (step->time() <= TMax)) {
      printStrawGasStep(step, "data", i);
    }
  }
 
}


//-----------------------------------------------------------------------------
void TAnaDump::printStepPointMC(const mu2e::StepPointMC* Step, const char* Detector, const char* Opt) {
  const char* oname = "TAnaDump::printStepPointMC";
    TString opt = Opt;

    if ((opt == "") || (opt.Index("banner") >= 0)) {
      printf("---------------------------------------------------------------------------------------------");
      printf("----------------------------");
      printf("--------------------------------------------------------------------------------------------------------------------\n");
      printf("  Vol          PDG    ID GenIndex PPdg ParentID      X          Y          Z          T      ");
      printf("  X0          Y0         Z0 ");
      printf("  Edep(Tot) Edep(NI)  Edep(I)    Step  EndCode  Energy    EKin     Mom       Pt    doca   Creation       StopProc   \n");
      printf("---------------------------------------------------------------------------------------------");
      printf("----------------------------");
      printf("--------------------------------------------------------------------------------------------------------------------\n");
    }

    mu2e::GeomHandle<mu2e::Tracker> ttHandle;
    const mu2e::Tracker* tracker = ttHandle.get();
  
    art::Ptr<mu2e::SimParticle> const& simptr = Step->simParticle();
    const mu2e::SimParticle* sim  = simptr.operator ->();
    if (sim == NULL) {
      printf(">>> ERROR: %s sim == NULL\n",oname);
    }

    art::Ptr<mu2e::SimParticle> const& parentptr = sim->parent();

    int parent_pdg_id (-1), parent_id(-1);

    const mu2e::SimParticle* par = parentptr.get();
    if (par != NULL) {
      parent_pdg_id = (int) par->pdgId();
      parent_id     = (int) par->id().asInt();
    }

    double doca = -9999.;
    if ((strcmp(Detector,"tracker") == 0) && tracker) {
      const mu2e::Straw* straw = &tracker->getStraw(mu2e::StrawId(Step->volumeId()));

      const CLHEP::Hep3Vector* v1 = &straw->getMidPoint();
      HepPoint p1(v1->x(),v1->y(),v1->z());

      const CLHEP::Hep3Vector* v2 = &Step->position();
      HepPoint    p2(v2->x(),v2->y(),v2->z());

      TrkLineTraj trstraw(p1,straw->getDirection()  ,0.,0.);
      TrkLineTraj trstep (p2,Step->momentum().unit(),0.,0.);

    // 2015-02-16 G. Pezzu and Murat change in the print out to be finished
    // 2015-02-25 P.Murat: fix sign - trajectory is the first !
    //  however, the sign of the disptance of closest approach is invariant
    // wrt the order
      TrkPoca poca(trstep, 0., trstraw, 0.);
    
      doca = poca.doca();
    }
    
    //    art::Ptr<mu2e::GenParticle> const& apgen = sim->genParticle();
    //    mu2e::GenParticle* gen = (mu2e::GenParticle*) &(*sim->genParticle());

    double mass = sim->startMomentum().m();

    double pabs = Step->momentum().mag();
    double energy = sqrt(pabs*pabs+mass*mass);
    double ekin  = energy-mass;
        
    CLHEP::Hep3Vector mom = Step->momentum();
    double pt = sqrt(pabs*pabs - mom.z()*mom.z());

    art::Handle<mu2e::PhysicalVolumeInfoMultiCollection> volumes;
    fEvent->getRun().getByLabel("g4run", volumes);

    double stepTime(-9999.);
    stepTime = Step->time();

    //    const mu2e::PhysicalVolumeInfo& pvinfo = volumes->at(sim->startVolumeIndex());
    //    const mu2e::PhysicalVolumeInfo& pvinfo = volumes->at(Step->volumeId()); - sometimes crashes..

    if ((opt == "") || (opt.Index("data") >= 0)) {
      printf("%5i %12i %6i %5i %5i %7i %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %8.2f %8.2f %8.2f %8.3f %4i %10.3f %8.3f %8.3f %8.3f %7.2f %-12s %-s\n",
	     (int) Step->volumeId(),
	     //	     pvinfo.name().data(), // smth is wrong with the name defined by volumeId()....
	     (int) sim->pdgId(),
	     (int) sim->id().asInt(),
	     (int) sim->generatorIndex(),
	     parent_pdg_id,
	     parent_id,
	     Step->position().x(),
	     Step->position().y(),
	     Step->position().z(),
	     stepTime,                             // Step->time(),
	     sim->startPosition().x(),
	     sim->startPosition().y(),
	     sim->startPosition().z(),
	     Step->totalEDep(),
	     Step->nonIonizingEDep(),
	     Step->ionizingEdep(),
	     Step->stepLength(),
	     Step->endProcessCode().id(),
	     energy,
	     ekin,
	     pabs,
	     pt,
	     doca,
	     sim->creationCode().name().data(),
	     Step->endProcessCode().name().data());
    }
}


//-----------------------------------------------------------------------------
void TAnaDump::printStepPointMCCollection(const char* ModuleLabel, 
					  const char* ProductName,
					  const char* ProcessName) {

  art::Handle<mu2e::StepPointMCCollection> handle;
  const mu2e::StepPointMCCollection*       coll(0);

  if (ProductName[0] != 0) {
    art::Selector  selector(art::ProductInstanceNameSelector(ProductName) &&
			    art::ProcessNameSelector        (ProcessName) && 
			    art::ModuleLabelSelector        (ModuleLabel)    );
    fEvent->get(selector, handle);
  }
  else {
    art::Selector  selector(art::ProcessNameSelector(ProcessName)         && 
			    art::ModuleLabelSelector(ModuleLabel)            );
    fEvent->get(selector, handle);
  }

  if (handle.isValid()) coll = handle.product();
  else {
    printf(">>> ERROR in TAnaDump::printStepPointMCCollection: failed to locate collection");
    printf(". BAIL OUT. \n");
    return;
  }

  int nsteps = coll->size();

  const mu2e::StepPointMC* step;


  int banner_printed = 0;
  for (int i=0; i<nsteps; i++) {
    step = &coll->at(i);
    if (banner_printed == 0) {
      printStepPointMC(step,ProductName,"banner");
      banner_printed = 1;
    }
    printStepPointMC(step,ProductName,"data");
  }
 
}

// //-----------------------------------------------------------------------------
// void TAnaDump::printStrawHitMCTruth(const mu2e::StrawHitMCTruth* Hit, const char* Opt) {
//   TString opt = Opt;
  
//   if ((opt == "") || (opt == "banner")) {
//     printf("--------------------------------------------------------------------\n");
//     printf(" Time Distance DistToMid         dt       eDep \n");
//     printf("--------------------------------------------------------------------\n");
//   }

//   if ((opt == "") || (opt == "data")) {
//     printf("%12.5f  %12.5f  %12.5f\n",
// 	   Hit->driftTime(),
// 	   Hit->driftDistance(),
// 	   Hit->distanceToMid());
//   }
// }


// //-----------------------------------------------------------------------------
// void TAnaDump::printStrawHitMCTruthCollection(const char* ModuleLabel, 
// 					      const char* ProductName,
// 					      const char* ProcessName) {

//   art::Handle<mu2e::StrawHitMCTruthCollection> shcHandle;
//   const mu2e::StrawHitMCTruthCollection*       shc;

//   if (ProductName[0] != 0) {
//     art::Selector  selector(art::ProductInstanceNameSelector(ProductName) &&
// 			    art::ProcessNameSelector(ProcessName)         && 
// 			    art::ModuleLabelSelector(ModuleLabel)            );
//     fEvent->get(selector, shcHandle);
//   }
//   else {
//     art::Selector  selector(art::ProcessNameSelector(ProcessName)         && 
// 			    art::ModuleLabelSelector(ModuleLabel)            );
//     fEvent->get(selector, shcHandle);
//   }

//   shc = shcHandle.product();

//   int nhits = shc->size();

//   const mu2e::StrawHitMCTruth* hit;


//   int banner_printed = 0;
//   for (int i=0; i<nhits; i++) {
//     hit = &shc->at(i);
//     if (banner_printed == 0) {
//       printStrawHitMCTruth(hit, "banner");
//       banner_printed = 1;
//     }
//     printStrawHitMCTruth(hit,"data");
//   }
 
// }

// //-----------------------------------------------------------------------------
// void TAnaDump::printTrackClusterMatch(const mu2e::TrackClusterMatch* Tcm, const char* Opt) {

//   TString opt = Opt;
  
//   if ((opt == "") || (opt == "banner")) {
//     printf("--------------------------------------------------------------------------------------\n");
//     printf("  Disk         Cluster          Track         chi2     du        dv       dt       E/P\n");
//     printf("--------------------------------------------------------------------------------------\n");
//   }

//   if ((opt == "") || (opt == "data")) {

//     const mu2e::CaloCluster*      cl  = Tcm->caloCluster();
//     const mu2e::TrkCaloIntersect* tex = Tcm->textrapol  ();

//     int disk     = cl->diskID();
//     double chi2  = Tcm->chi2();

//     printf("%5i %16p  %16p  %8.3f %8.3f %8.3f %8.3f %8.3f\n",
// 	   disk,  static_cast<const void*>(cl),  static_cast<const void*>(tex),  chi2,Tcm->du(),Tcm->dv(),Tcm->dt(),Tcm->ep());
//   }
// }



// //-----------------------------------------------------------------------------
// void TAnaDump::printTrackClusterMatchCollection(const char* ModuleLabel, 
// 						const char* ProductName,
// 						const char* ProcessName) {

//   printf(">>>> ModuleLabel = %s\n",ModuleLabel);

//   art::Handle<mu2e::TrackClusterMatchCollection> handle;
//   const mu2e::TrackClusterMatchCollection*       coll;

//   art::Selector  selector(art::ProductInstanceNameSelector(ProductName) &&
// 			  art::ProcessNameSelector(ProcessName)         && 
// 			  art::ModuleLabelSelector(ModuleLabel)            );

//   fEvent->get(selector,handle);

//   if (handle.isValid()) coll = handle.product();
//   else {
//     printf(">>> ERROR in TAnaDump::printTrackClusterMatchCollection: failed to locate requested collection. Available:");

//     // vector<art::Handle<mu2e::TrackClusterMatchCollection>> list_of_handles;
//     auto list_of_handles = fEvent->getMany<mu2e::TrackClusterMatchCollection>();

//     for (auto ih=list_of_handles.begin(); ih<list_of_handles.end(); ih++) {
//       printf("%s\n", ih->provenance()->moduleLabel().data());
//     }

//     printf(". BAIL OUT. \n");
//     return;
//   }

//   int nm = coll->size();

//   const mu2e::TrackClusterMatch* obj;

//   int banner_printed = 0;

//   for (int i=0; i<nm; i++) {
//     obj = &coll->at(i);
//     if (banner_printed == 0) {
//       printTrackClusterMatch(obj, "banner");
//       banner_printed = 1;
//     }
//     printTrackClusterMatch(obj,"data");
//   }
 
// }


//-----------------------------------------------------------------------------
// void TAnaDump::refitTrack(void* Trk, double NSig) {
//   KalRep* trk = (KalRep*) Trk;

//   const TrkHitVector* hits = &trk->hitVector();

//   for (auto it=hits->begin(); it!=hits->end(); it++) {

//     // TrkStrawHit inherits from TrkHitOnTrk

//     TrkHit* hit = (TrkHit*) &(*it);

//     const mu2e::TrkStrawHit* straw_hit = (const mu2e::TrkStrawHit*) (*it);

//     double res = straw_hit->resid();

//     if (fabs(res) > 0.1*NSig) {
//       trk->deactivateHit(hit);
//     if (fabs(res) > 0.1*NSig) {
//       trk->deactivateHit(hit);
//     }
//   }

//   trk->fit();

//   printKalRep(trk, "hits");
// }


//-----------------------------------------------------------------------------
// emulate calculation of the unbiased residual
//-----------------------------------------------------------------------------
// void TAnaDump::Test_000(const KalRep* Krep) { // , mu2e::TrkStrawHit* Hit) {

//  apparently, Hit has (had ?) once to be on the track ?

  // double             s, slen, rdrift, sflt, tflt, doca/*, sig , xdr*/;
  // const mu2e::Straw *straw;
  // int                sign /*, shId, layer*/;
  // HepPoint           spi , tpi , hpos;
 
  // CLHEP::Hep3Vector  spos, sdir;
  // TrkSimpTraj  *ptraj(NULL);

  // fTmp[0] = -1;
  // fTmp[1] = -1;


//   KalRep* krep = Krep->clone();

//   straw  = &Hit->straw();
//   //  layer  = straw->id().getLayer();
//   rdrift = Hit->driftRadius();
//   //  shId   = straw->index().asInt();
  
//   //  const KalHit* khit = krep->findHotSite(Hit);

//   s      = Hit->fltLen();

//   //  int active = Hit->isActive();

// //   if (active) krep->deactivateHot(Hit);

// //   krep->resetFit();
// //   krep->fit();
// // 					// local track trajectory
// //   ptraj = krep->localTrajectory(s,slen);

//   vector<KalSite*>::const_iterator itt;
//   int found = 0;
//   for (auto /* vector<KalSite*>::const_iterator */ it=krep->siteList().begin();
//        it!= krep->siteList().end(); it++) {
//     const KalHit* kalhit = (*it)->kalHit();
//     if (kalhit && (kalhit->hitOnTrack() == Hit)) {
//       itt   = it;
//       found = 1;
//       break;
//     }
//   }

//   trk->fit();

//   printKalRep(trk, "hits");
// }


// //-----------------------------------------------------------------------------
// // emulate calculation of the unbiased residual
// //-----------------------------------------------------------------------------
// void TAnaDump::Test_000(const KalRep* Krep, mu2e::TrkStrawHit* Hit) {

// //  apparently, Hit has (had ?) once to be on the track ?

//   // double             s, slen, rdrift, sflt, tflt, doca/*, sig , xdr*/;
//   // const mu2e::Straw *straw;
//   // int                sign /*, shId, layer*/;
//   // HepPoint           spi , tpi , hpos;
 
//   // CLHEP::Hep3Vector  spos, sdir;
//   // TrkSimpTraj  *ptraj(NULL);

//   fTmp[0] = -1;
//   fTmp[1] = -1;

// //   KalRep* krep = Krep->clone();

// //   straw  = &Hit->straw();
// //   //  layer  = straw->id().getLayer();
// //   rdrift = Hit->driftRadius();
// //   //  shId   = straw->index().asInt();
  
// //   //  const KalHit* khit = krep->findHotSite(Hit);

// //   s      = Hit->fltLen();

// //   //  int active = Hit->isActive();

// // //   if (active) krep->deactivateHot(Hit);

// // //   krep->resetFit();
// // //   krep->fit();
// // // 					// local track trajectory
// // //   ptraj = krep->localTrajectory(s,slen);

// //   vector<KalSite*>::const_iterator itt;
// //   int found = 0;
// //   for (auto /* vector<KalSite*>::const_iterator */ it=krep->siteList().begin();
// //        it!= krep->siteList().end(); it++) {
// //     const KalHit* kalhit = (*it)->kalHit();
// //     if (kalhit && (kalhit->hitOnTrack() == Hit)) {
// //       itt   = it;
// //       found = 1;
// //       break;
// //     }
// //   }
      
// //   if (found == 0) {
// //     ptraj = (TrkSimpTraj  *) krep->localTrajectory(s,slen);
// //   }
// //   else {
// //     krep->smoothedTraj(itt,itt,ptraj);
// //   }

// //   spos = straw->getMidPoint();
// //   sdir = straw->getDirection();
// // 					// convert Hep3Vector into HepPoint

// //   HepPoint    p1(spos.x(),spos.y(),spos.z());

// // 					// wire as a trajectory
// //   TrkLineTraj st(p1,sdir,0.,0.);

// //   TrkPoca poca = TrkPoca(st,0.,*ptraj,0);

// //   Hep3Vector        sdi , tdi, u;

// //   sflt = poca.flt1();
// //   tflt = poca.flt2();

// //   st.getInfo(sflt,spi,sdi);
// //   ptraj->getInfo(tflt,tpi,tdi);
      
// //   u    = sdi.cross(tdi).unit();  // direction towards the center

// //   sign     = Hit->ambig();
// //   hpos     = spi+u*rdrift*sign;
// // 					// hit residal is positive when its residual vector 
// // 					// points inside
// //   doca     = (tpi-hpos).dot(u);
// //   //  sig      = sqrt(rdrift*rdrift +0.1*0.1); // 2.5; // 1.; // hit[ih]->hitRms();
// //   //  xdr      = doca/sig;

// //   fTmp[0]  = doca;

//   //  if (active) krep->activateHot(Hit);

// }
