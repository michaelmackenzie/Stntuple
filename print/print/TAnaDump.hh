//
#ifndef __murat_inc_TAnaDump_hh__
#define __murat_inc_TAnaDump_hh__

#include "TObject.h"
#include "TObjArray.h"
#include "TString.h"
#include "TGraph.h"
#include "TMarker.h"
#include "TCanvas.h"
#include "TEllipse.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"

#ifndef __CLING__

#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/Name.h"
#include "fhiclcpp/types/Comment.h"
#include "fhiclcpp/ParameterSet.h"

#include "art/Framework/Principal/Event.h"

#else

namespace art {
  class Event;
}

namespace fhicl {
  class ParameterSet;
}
#endif

#include "Offline/MCDataProducts/inc/CaloMCTruthAssns.hh"
#include "Offline/MCDataProducts/inc/StrawDigiMC.hh"

#include "Offline/RecoDataProducts/inc/ComboHit.hh"
#include "Offline/RecoDataProducts/inc/HelixHit.hh"

namespace mu2e {

#ifndef MCDataProducts_StrawDigiMC_hh
  class StrawDigiMCCollection;
#endif

  class StrawHit;
  class CaloCluster;
  class CaloProtoCluster;
  class CrvDigi;
  class CrvRecoPulse;
  // class CrvCoincidence;
  class CrvCoincidenceCluster;
  // class TrkToCaloExtrapol;
  class StepPointMC;
  class StrawGasStep;
  class GenParticle;
  class SimParticle;
  class TimeCluster;
  class KalSeed;
  class ComboHit;
  class CosmicTrackSeed;
  class HelixSeed;
  // class TrackClusterMatch;
  // class TrkPrintUtils;
  // class TrkCaloHit;
  // class TrkStrawHit;
}

class KalRep;

class TAnaDump : public TObject {
public:
#ifndef __CLING__
  struct Config {
    using Name    = fhicl::Name;
    using Comment = fhicl::Comment;
    fhicl::Atom<int>                   interactiveMode{Name("interactiveMode"), Comment("1: interactive mode"  ) };
    fhicl::Atom<std::string>           rootMacro      {Name("rootMacro"      ), Comment("good hit mask"        ) };
    fhicl::Table<fhicl::ParameterSet>  debugBits      {Name("debugBits"      ), Comment("debug bits"           ) };
    // fhicl::Table<TrkReco>         printUtils    (Name("printUtils"       ), Comment("print Utils"   ) );
  };
#endif

  const art::Event*                  fEvent;
  TObjArray*                         fListOfObjects;
  TString                            fFlagBgrHitsModuleLabel;
  art::InputTag                      fSdmcCollTag;
  const mu2e::StrawDigiMCCollection* _mcdigis;
  double                             fTmp[100];  // for testing

  // mu2e::TrkPrintUtils*               _printUtils;

private:

  TAnaDump(const fhicl::ParameterSet*  Pset = NULL);
  // TAnaDump(const fhicl::Table<Config>& config     );
  ~TAnaDump();
  
  class  Cleaner {
  public: 
    Cleaner();
    ~Cleaner();
  };
  
  friend class Cleaner;

  static TAnaDump*  fgInstance;
public:
//-----------------------------------------------------------------------------
// TAnaDump gets initialized by the first TModule instantiated
//-----------------------------------------------------------------------------
  static TAnaDump*  Instance(const fhicl::ParameterSet* PSet = NULL);
//-----------------------------------------------------------------------------
// accessors
//-----------------------------------------------------------------------------
  const art::Event*              Event      () { return fEvent      ; }
//-----------------------------------------------------------------------------
// other methods
//-----------------------------------------------------------------------------
  void   AddObject      (const char* Name, void* Object);
  void*  FindNamedObject(const char* Name);

  void   SetEvent(const art::Event* Evt) { fEvent = Evt; }

  void   SetFlagBgrHitsModuleLabel(const char*    Tag) { fFlagBgrHitsModuleLabel = Tag; }
  void   SetStrawDigiMCCollTag    (art::InputTag& Tag) { fSdmcCollTag            = Tag; }

  double evalWeight(const mu2e::ComboHit*      Hit       ,
		    CLHEP::Hep3Vector&         StrawDir  ,
		    CLHEP::Hep3Vector&         HelCenter , 
		    double                     Radius    ,
		    int                        WeightMode,
		    fhicl::ParameterSet const& Pset      );

  void   evalHelixInfo(const mu2e::HelixSeed*  Helix,
		       int&                    NLoops,
		       int&                    NHitsLoopFailed);

  void   printEventHeader();
//-----------------------------------------------------------------------------
// calorimeter
//-----------------------------------------------------------------------------
  void printCalorimeter();

  void printCaloCrystalHits (const char* ModuleLabel, 
			     const char* ProductName = "", 
			     const char* ProcessName = ""); 

  void printCaloDigiCollection(const char* ModuleLabel, 
			       const char* ProductName = "", 
			       const char* ProcessName = ""); 

  void printCaloRecoDigiCollection(const char* ModuleLabel, 
				   const char* ProductName = "", 
				   const char* ProcessName = "");

  void printCaloHits        (const char* ModuleLabel, 
			     const char* ProductName = "", 
			     const char* ProcessName = ""); 

  void printCaloCluster          (const mu2e::CaloCluster* Cluster ,
				  const char*              Opt = "",
				  const mu2e::CaloHitMCTruthAssn* CaloHitTruth=NULL);
  
  void printCaloClusterCollection (const char* ModuleLabel, 
				   const char* ProductName= "",
				   const char* ProcessName= "",
				   double      Emin=50.,
				   int         hitOpt=0,
				   const char* MCModuleLabel= "");
  
  void printCaloProtoCluster      (const mu2e::CaloProtoCluster* Clu     ,
				   const char*                   Opt = "");
  
  void printCaloProtoClusterCollection (const char* ModuleLabel, 
					const char* ProductName,
					const char* ProcessName);
//-----------------------------------------------------------------------------
// CRV
//-----------------------------------------------------------------------------
  // void printCrvCoincidence        (const mu2e::CrvCoincidence* CrvC  ,
  //       			   const char*                 Opt = "");
  
  // void printCrvCoincidenceCollection (const char* ModuleLabel, 
  //       			      const char* ProductName= "",
  //       			      const char* ProcessName= "");

  void printCrvCoincidenceCluster (const mu2e::CrvCoincidenceCluster* CrvC  ,
				   const char*                        Opt = "");
  
  void printCrvCoincidenceClusterCollection (const char* ModuleLabel, 
					     const char* ProductName= "",
					     const char* ProcessName= "");

  void printCrvRecoPulse          (const mu2e::CrvRecoPulse* Pulse  ,
				   const char*              Opt = "");
  
  void printCrvRecoPulseCollection (const char* ModuleLabel, 
				    const char* ProductName= "",
				    const char* ProcessName= "");

  void printCrvDigi          (const mu2e::CrvDigi* Digi  ,
				   const char*              Opt = "");
  
  void printCrvDigiCollection (const char* ModuleLabel, 
				    const char* ProductName= "",
				    const char* ProcessName= "");
//-----------------------------------------------------------------------------
// tracking
//-----------------------------------------------------------------------------
  void printComboHit      (const mu2e::ComboHit*     Hit, 
			   const mu2e::StrawGasStep* Step,
			   const char*               Opt   = "", 
			   int                       INit  = -1,
			   int                       Flags = -1);
  
  void printComboHitCollection (const char* StrawHitCollTag, 
				const char* StrawDigiMCCollTag = "",  // "makeSD" or "compressDigiMCs"
				double TMin = -1.e6, double TMax = 1.e6,
                                double EMin = -1.e6, double EMax = 1.e6);
 
  void printCosmicTrackSeed    (const mu2e::CosmicTrackSeed* CTSeed    , 
				const char* StrawHitCollTag            ,  // usually - "makeSH"
				const char* StrawDigiCollTag = "makeSD",
				const char* Opt              = ""      );

  void printHelixSeed          (const mu2e::HelixSeed*         Helix   , 
				//				const char* HelixSeedCollTag           ,
				const char* StrawHitCollTag            ,  // usually - "makeSH"
				const char* StrawDigiCollTag = "makeSD",
				const char* Opt              = ""      );

  void printCosmicTrackSeedCollection(const char* CTSCollTag             ,  // always needed
				int         PrintHits          = 0       ,
				const char* StrawHitCollTag    = "makeSH",  // usually, "makeSH"
				const char* StrawDigiMCCollTag = "compressDigiMCs" ); // "makeSD" or "compressDigiMCs"

  void printHelixSeedCollection(const char* HelixSeedCollTag             ,  // always needed
				int         PrintHits          = 0       ,
				const char* StrawHitCollTag    = "makeSH",  // usually, "makeSH"
				const char* StrawDigiMCCollTag = "compressDigiMCs" ); // "makeSD" or "compressDigiMCs"

  void printHelixHit      (const mu2e::HelixHit*     HelHit,
			   const mu2e::ComboHit*     Hit, 
			   const mu2e::StrawGasStep* Step,
			   const char*               Opt   = "", 
			   int                       INit  = -1,
			   int                       Flags = -1);
  
  void printKalSeed_Line        (const mu2e::KalSeed* Seed                   , 
                                 const char* Opt         = ""                ,
                                 const char* ShCollTag   = "makeSH"          ,
                                 const char* SdmcCollTag = "compressDigiMCs"); // "makeSD" 

  void printKalSeed            (const mu2e::KalSeed* Seed                      , 
				const char* Opt                = ""               ,
				const char* StrawHitCollTag    = "makeSH"         ,
				const char* StrawDigiMCCollTag = "compressDigiMCs");

  void printKalSeedCollection  (const char* CollTag                               ,
				int         hitOpt             = 0                ,
				const char* StrawHitCollTag    = "makeSH"         ,
				const char* StrawDigiMCCollTag = "compressDigiMCs");

  // void printKalRep(const KalRep* Krep, const char* Opt = "", const char* Prefix = "");

  // void printKalRepCollection(const char* KalRepCollTag               , 
  //       		     int         hitOpt             = 0      , 
  //       		     const char* StrawDigiMCCollTag = nullptr); 
//-----------------------------------------------------------------------------
// MC truth: gen and sim particles
//-----------------------------------------------------------------------------
  void printGenParticle   (const mu2e::GenParticle*    P  , const char* Opt = "");

  void printGenParticleCollections();

  void printSimParticle   (const mu2e::SimParticle*    P  , const char* Opt = "", const void* PrintData = nullptr);

  void printSimParticleCollection(const char* ModuleLabel     , 
				  const char* ProductName = "", 
				  const char* ProcessName = "");
  
  void printSimParticleCollection(const art::InputTag& SimpCollTag);
//-----------------------------------------------------------------------------
// pass the detector name to know what to print for different detectors
// tested for Detector = 'tracker', 'calorimeter'
//-----------------------------------------------------------------------------
  void printStepPointMC(const mu2e::StepPointMC* Step, const char* Detector, const char* Opt = "");

  void printStepPointMCCollection (const char* ModuleLabel     , 
				   const char* ProductName = "", 
				   const char* ProcessName = "");

  void printStrawHit      (const mu2e::StrawHit*     Hit, 
			   const mu2e::StrawGasStep* Step,
			   const char*               Opt   = "", 
			   int                       INit  = -1,
			   int                       Flags = -1);
  
  void printStrawHitCollection (const char* StrawHitCollTag, 
				const char* StrawDigiMCCollTag = "compressDigiMCs", 
				double TMin = -1.e6, double TMax =  1.e6);

  void printStrawGasStep   (const mu2e::StrawGasStep* Step     ,
			    const char*               Opt  = "",
			    int   IStep                    = -1);
  
  void printStrawGasStepCollection (const char* CollTag, double TMin = -1.e6, double TMax =  1.e6);

//-----------------------------------------------------------------------------
// time clusters
//-----------------------------------------------------------------------------
  void printTimeCluster   (const mu2e::TimeCluster*            TimePeak        , 
                           const char*                         Opt         = "", 
			   const mu2e::ComboHitCollection*     ChColl      = nullptr,
			   const char*                         SdmcCollTag = "makeSD");

  void printTimeClusterCollection(const char* TcCollTag             ,    // time cluster collection tag
				  const char* ChCollTag             ,    // combo hit coll tag
				  int         PrintHits   = 0       ,
				  const char* SdmcCollTag = nullptr);   // straw digi mc coll tag
//-----------------------------------------------------------------------------
// calorimeter cluster added to the track fit
//-----------------------------------------------------------------------------
  // void printTrkCaloHit(const KalRep* Krep, mu2e::TrkCaloHit* CaloHit);
//-----------------------------------------------------------------------------
// extrapolation and track-to-calorimeter matching
//-----------------------------------------------------------------------------
  // void printTrkToCaloExtrapol           (const mu2e::TrkToCaloExtrapol*extrk,
  //       				 const char* Opt = "");

  // void printTrkToCaloExtrapolCollection (const char* ModuleLabel, 
  //       				 const char* ProductName = "", 
  //       				 const char* ProcessName = "");

  // void printTrackClusterMatch           (const mu2e::TrackClusterMatch* TcMatch, const char* Option);


  // void printTrackClusterMatchCollection(const char* ModuleLabel     , 
  //       				const char* ProductName = "", 
  //       				const char* ProcessName = "");
  
  //       				// refit track dropping hits away > NSig sigma (0.1)
  // void  refitTrack(void* Trk, double NSig);

  // void  Test_000(const KalRep* Krep, mu2e::TrkStrawHit* Hit);

  ClassDef(TAnaDump,0)
};


#endif
