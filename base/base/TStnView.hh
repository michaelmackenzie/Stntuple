///////////////////////////////////////////////////////////////////////////////
// 2D view 
///////////////////////////////////////////////////////////////////////////////
#ifndef Stntuple_base_TStnView_hh
#define Stntuple_base_TStnView_hh

#include "TMarker.h"
#include "TNamed.h"
#include "TObjArray.h"
#include "TGaxis.h"
#include "TVector3.h"

#include "Stntuple/base/TVisNode.hh"

class TStnView: public TNamed {
protected:
  int                 fType;            // view type
  int                 fIndex;           // for calorimeter - 2 views, for example
  void*               fMother;          // non-null. if in the local ref system of some object

  static int          fgDebugLevel;
  
  Int_t               fPx1;
  Int_t               fPy1;
  Int_t               fPx2;
  Int_t               fPy2;
					// displayed range in world coordinates
  double              fXMin;
  double              fXMax;
  double              fYMin;
  double              fYMax;

  TGaxis*             fXAxis;
  TGaxis*             fYAxis;

  TMarker*            fCenter;

  int                 fCursorX;
  int                 fCursorY;

  TObjArray*          fListOfNodes;	// list of nodes to visualize

protected:
  float     fTMin;
  float     fTMax;

public:
  TStnView(int Type = -1, int Index = -1); 

  TStnView(int Type, int Index, const char* Name, const char* Title);

  virtual ~TStnView();
//-----------------------------------------------------------------------------
// accessors
//-----------------------------------------------------------------------------
  int           Type () { return fType;  }
  int           Index() { return fIndex; }

  void*         GetMother()      { return fMother; }
  int           GetNNodes()      { return fListOfNodes->GetEntriesFast(); }
  TVisNode*     GetNode  (int I) { return (TVisNode*) fListOfNodes->UncheckedAt(I);   }
  TObjArray*    GetListOfNodes() { return fListOfNodes; }
  int           GetPx1 ()        { return fPx1;  }
  int           GetPx2 ()        { return fPx2;  }
  int           GetPy1 ()        { return fPy1;  }
  int           GetPy2 ()        { return fPy2;  }
  double        GetYMin()        { return fYMin; }
  double        GetYMax()        { return fYMax; }

  void          AddNode(TVisNode* Node) { fListOfNodes->Add(Node); }
//-----------------------------------------------------------------------------
// setters
//-----------------------------------------------------------------------------
  void          SetDebugLevel (int Level);               // *MENU*
  void          SetIndex      (int Index) { fIndex = Index; } 

  void          SetMinEDep    (float E);                 // *MENU* 
  void          SetMaxEDep    (float E);                 // *MENU* 
  void          SetMother     (void* Mother) { fMother = Mother; };
  void          SetStations   (int I1, int I2);          // *MENU* 
  void          SetTimeWindow (float TMin, float TMax);  // *MENU* 
  void          SetType       (int Type ) { fType  = Type;  } 
//-----------------------------------------------------------------------------
// overloaded functions of TObject
//-----------------------------------------------------------------------------
  virtual void  Paint               (Option_t* option = "");
  virtual Int_t DistancetoPrimitive (Int_t px, Int_t py);
  virtual void  ExecuteEvent        (Int_t event, Int_t px, Int_t py);
  virtual char* GetObjectInfo       (Int_t Px, Int_t Py) const ;

  virtual void  Print               (Option_t* option = "") const ;  // *MENU* 

  ClassDef(TStnView,0)
};

#endif
