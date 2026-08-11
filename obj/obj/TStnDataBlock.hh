#ifndef STNTUPLE_TStnDataBlock
#define STNTUPLE_TStnDataBlock
//-----------------------------------------------------------------------------
//  definition of the STNTUPLE track block
//  Author:    Pasha Murat (CDF/FNAL)
//  Date:      Nov 10 2000
// 
//  base class for STNTUPLE data block
//-----------------------------------------------------------------------------
#include "Stntuple/obj/AbsEvent.hh"
#include "Stntuple/obj/TStnInitDataBlock.hh"

#include "TNamed.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TBuffer.h"

class TStnNode;
class TStnEvent;

class TStnDataBlock: public TObject {
public:
  static const Float_t    kUndefined;
protected:
//-----------------------------------------------------------------------------
//  data members
//-----------------------------------------------------------------------------
  Int_t    f_EventNumber;		// !
  Int_t    f_RunNumber;			// !
  Int_t    f_SubrunNumber;		// !
  Int_t    fLinksInitialized;		// !
  Int_t    fUserInitialization;		// ! if != 0, Init methods are called
					// by StntupleMakerModule
  TStnNode* fNode;			// ! 

					// externally defined initialization 
					// function

  Int_t (*fExternalInit)(TStnDataBlock* block,AbsEvent* event,Int_t mode); // !

					// externally defined link resolution
					// function

  Int_t (*fResolveLinks)(TStnDataBlock* block,AbsEvent* event,Int_t mode); // !

  Int_t    fInitMode;			// ! initialization mode (user-defined)

  TString  fCollName;			// ! name of the corresponding 
					//   collection (don't write it out)

  TObjArray* fListOfCollNames;          // ! list of TNamed objects name:title
					//   name is a class name, 
					//   title contains the rest

  TStnEvent* fEvent;			// ! backward pointer to the event
  TObjArray* fMessageList;		// ! 
  Int_t      fCurrentEntry;		// ! last read entry in the chain
  Int_t      fValid;			// ! 1, if properly initialized

  TStnInitDataBlock*  fInitBlock;       // ! coming replacement for fExternalInit
//-----------------------------------------------------------------------------
//  functions
//-----------------------------------------------------------------------------
protected:
					// init functions to be overloaded
					// by the derived classes

  virtual Int_t fOverloadedInit(AbsEvent* event, Int_t mode) { return 0; }

public:
					// ****** constructors and destructor
  TStnDataBlock();
  virtual ~TStnDataBlock();
					// ****** init methods

  Int_t Init(AbsEvent* event, int mode) {
    if   (fInitBlock   ) return fInitBlock->InitDataBlock(this,event,mode);
    else {
      if (fExternalInit) return fExternalInit  (this,event,mode);
      else               return fOverloadedInit(event,mode);
    }
  }

  Int_t ResolveLinks(AbsEvent* event, int mode) {
    if   (fInitBlock   ) return fInitBlock->ResolveLinks(this,event,mode);
    else {
      if (fResolveLinks) return fResolveLinks (this,event,mode);
      else               return 0;
    }
  }
					// return 1 if block has already been
					// initialized

  Int_t Initialized(int event, int run, int Subrun = -1) {
    return ((event == f_EventNumber) && (run == f_RunNumber) && (Subrun == f_SubrunNumber));
  }

//-----------------------------------------------------------------------------
//  accessors
//-----------------------------------------------------------------------------
  Int_t       UserInitialization() { return fUserInitialization; }
  Int_t       LinksInitialized  () { return fLinksInitialized;   }
  Int_t       InitMode          () { return fInitMode;           }
  TString*    CollName          () { return &fCollName;          }
  TStnNode*   GetNode           () { return fNode;               }
  TStnEvent*  GetEvent          () { return fEvent;              }
  Int_t       GetValid          () { return fValid;              }
  Bool_t      IsValid           () { return fValid==1;           }
  TObjArray*  MessageList       () { return fMessageList;        }
  Int_t       EventNumber       () { return f_EventNumber;       }
  Int_t       RunNumber         () { return f_RunNumber;         }

  void GetCollTag    (const char* CollectionClassName, char* CollTag    );
  void GetModuleLabel(const char* CollectionClassName, char* ModuleLabel);
  void GetDescription(const char* CollectionClassName, char* Description);
  void GetProcessName(const char* CollectionClassName, char* ProcessName);
  void GetValue      (const char* Key                , char* Value      );
//-----------------------------------------------------------------------------
//  setters
//-----------------------------------------------------------------------------
  void  SetUserInitialization(Int_t m) { fUserInitialization = m; }
  void  SetLinksInitialized         () { fLinksInitialized   = 1; }
  void  SetNode       (TStnNode* node) { fNode  = node; }
  void  SetEvent      (TStnEvent*  ev) { fEvent = ev; }
  void  SetValid      (Int_t    Valid) { fValid = Valid; }

  void  SetInitBlock  (TStnInitDataBlock* InitBlock) { fInitBlock = InitBlock; }

  TNamed*  AddCollName(const char* CollName         , 
		       const char* CollTag          , 
		       const char* Description = "" ,
		       const char* ProcessName = "" );

  void  SetExternalInit(Int_t (*f)(TStnDataBlock*, AbsEvent*, Int_t)) { 
    fExternalInit = f;
  }

  void  SetResolveLinksMethod(Int_t (*f)(TStnDataBlock*, AbsEvent*, Int_t)) { 
    fResolveLinks = f;
  }

  void  SetInitMode            (int m) { fInitMode = m; }
  void  SetCollName (const char* name) { fCollName = name; }

  void  SetCollName (const char* Process, 
		     const char* Description, 
		     const char* CollType = 0);

  void  AddMessage  (const char* mess) { 
    fMessageList->Add(new TObjString(mess));
  }

  virtual Int_t GetEntry(Int_t ientry);
					// ****** overloaded functions of 
					// TObject
  void Print(Option_t* opt="") const;

  ClassDef(TStnDataBlock,1)
};
#endif
