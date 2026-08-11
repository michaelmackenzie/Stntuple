///////////////////////////////////////////////////////////////////////////////
//
// $Author: murat $
// $Date: 2014/09/20 17:54:06 $
//
// Contact person:  PM
//
///////////////////////////////////////////////////////////////////////////////

// C++ includes.
#include <map>
#include <iostream>
#include <string>

// Framework includes.
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art_root_io/TFileService.h"
#include "art/Framework/Principal/Selector.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

// Mu2e includes.

// ROOT includes
#include "TApplication.h"
#include "TDirectory.h"

#include "Stntuple/print/TAnaDump.hh"
#include "Stntuple/mod/TModule.hh"

using namespace std;
using CLHEP::Hep3Vector;

namespace mu2e {

  class StntupleEventDump : public TModule {
  private:
//-----------------------------------------------------------------------------
// Input parameters: Module labels 
//-----------------------------------------------------------------------------
    std::string        _moduleLabel;	             // this module label
    std::string        _processName;

    std::string        _producerName;
//-----------------------------------------------------------------------------
// end of input parameters
// Options to control the display
// hit flag bits which should be ON and OFF
//-----------------------------------------------------------------------------
    TApplication*                               fApplication;

  public:

    explicit StntupleEventDump(fhicl::ParameterSet const& pset);
    virtual ~StntupleEventDump();

    int      getData(const art::Event* Evt);
    void     Init   (art::Event* Evt);
//-----------------------------------------------------------------------------
// overloaded virtual methods of the base class
//-----------------------------------------------------------------------------
    virtual void     beginJob();
    virtual void     beginRun(const art::Run& aRun);
    virtual void     analyze (const art::Event& Evt);
  };


//-----------------------------------------------------------------------------
  StntupleEventDump::StntupleEventDump(fhicl::ParameterSet const& pset) :
    TModule(pset,pset.get<fhicl::ParameterSet>("TModule"), "StntupleEventDump")
  {
    fApplication = 0;
  }

//-----------------------------------------------------------------------------
  StntupleEventDump::~StntupleEventDump() {
    if (fApplication) delete fApplication;
  }


//-----------------------------------------------------------------------------
  void StntupleEventDump::beginJob() {
    //     const char oname[] = "StntupleEventDump::beginJob";
    int    tmp_argc(0);
    char** tmp_argv(0);

    if (!gApplication) {
      fApplication = new TApplication("StntupleEventDump_module", &tmp_argc, tmp_argv);
    }
  }

//-----------------------------------------------------------------------------
  void StntupleEventDump::beginRun(const art::Run& Run) {
  }

//-----------------------------------------------------------------------------
// get data from the event record
//-----------------------------------------------------------------------------
  int StntupleEventDump::getData(const art::Event* Evt) {
    //    const char* oname = "StntupleEventDump::getData";
    return 0;
  }

//-----------------------------------------------------------------------------
  void StntupleEventDump::analyze(const art::Event& Evt) {
    //    const char* oname = "StntupleEventDump::filter";
//-----------------------------------------------------------------------------
// go into interactive mode, till '.q' is pressed
//-----------------------------------------------------------------------------
    TModule::analyze(Evt);
  } 
}

using mu2e::StntupleEventDump;
DEFINE_ART_MODULE(StntupleEventDump)
