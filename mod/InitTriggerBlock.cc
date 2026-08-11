///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

#include "Stntuple/mod/InitTriggerBlock.hh"
#include "Stntuple/obj/TStnDBManager.hh"
#include "Stntuple/obj/TStnTriggerTable.hh"

#include "Offline/Mu2eUtilities/inc/TriggerResultsNavigator.hh"

//-----------------------------------------------------------------------------
// in this case AbsEvent is just not used
//-----------------------------------------------------------------------------
int StntupleInitTriggerBlock::InitDataBlock(TStnDataBlock* Block, AbsEvent* Event, int Mode) {

  int       ev, rn, sr; 

  ev = Event->event();
  rn = Event->run();
  sr = Event->subRun();

  if (Block->Initialized(ev,rn,sr)) return 0;
  
  TStnTriggerBlock* block = (TStnTriggerBlock*) Block;
//-----------------------------------------------------------------------------
// number of paths is variable, but paths have predefined bit numbers assigned
//-----------------------------------------------------------------------------
  block->f_EventNumber  = ev;
  block->f_RunNumber    = rn;
  block->f_SubrunNumber = sr;
  block->fNPaths        = 0;
  block->fPaths.Init(fNTriggerBits);

  const art::TriggerResults*       tr(nullptr);
  art::Handle<art::TriggerResults> trH; 

  bool ok = Event->getByLabel<art::TriggerResults>(fTriggerResultsTag,trH);

  if (ok) { 
//-----------------------------------------------------------------------------
// trigger results are present, in some form, fill TStnTriggerBlock
//-----------------------------------------------------------------------------
    tr = trH.product();
    mu2e::TriggerResultsNavigator trn(tr);
  
    TStnDBManager*    dbm  = TStnDBManager::Instance();
    TStnTriggerTable* ttbl = (TStnTriggerTable*) dbm->GetTable("TriggerTable");

    TObjArray list(500);

    block->fNPaths = trn.getTrigPaths().size();
    // block->fPaths.Init(nbits);

    for (int i=0; i<block->fNPaths; i++) {
      const std::string& name = "" ; // FIXME trn.getTrigPathNameByIndex(i);

      if (trn.accepted(name)) {
        //-----------------------------------------------------------------------------
        // the event has passed the logic of the trigger path named 'name'
        //-----------------------------------------------------------------------------
        char oname[] = "StntupleInitTriggerBlock::InitDataBlock";

        int trig_bit = -1;
        try {
          trig_bit = -1; // FIXME PM trn.getTrigBitByName(name);
        } catch (...) {}
        // ttbl->GetListOfTriggers(name.data(),&list);
        ttbl->GetListOfTriggers("",&list); // check against all triggers
        int nt = list.GetEntriesFast();
        bool found = false;
        for (int k=0; k<nt; k++){
          TStnTrigger* tr = (TStnTrigger*) list.At(k);
          const int bit = tr->Bit();
          if (tr->Name().Data() == name || (trig_bit >= 0 && bit == trig_bit)) {
            block->fPaths.SetBit((trig_bit >= 0) ? trig_bit : bit); // use the bit from the TriggerResults
            found = true;
            break;
          }
        }
        if (!found && !std::isupper(name[0])) { // ignore paths that start with uppercase letters, these are usually Offline paths
          printf("%s ERROR: path %s is not in the trigger table for this run. The bit is NOT SET\n",oname,name.data());
        }
      }
    }
  }

  return 0;
}


//-----------------------------------------------------------------------------
// keep this function as an example, don't really need it
//-----------------------------------------------------------------------------
int StntupleInitTriggerBlock::ResolveLinks(TStnDataBlock* Block, AbsEvent* Event, int Mode) {

  int ev, rn, sr;
  
  ev = Event->event();
  rn = Event->run();
  sr = Event->subRun();
  
  if (! Block->Initialized(ev,rn,sr)) return -1;

  return 0;
}
