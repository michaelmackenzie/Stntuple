#ifndef STNTUPLE_TCrvClusterBlock
#define STNTUPLE_TCrvClusterBlock

#include "TClonesArray.h"

#include "Stntuple/obj/TStnLinkBlock.hh"
#include "Stntuple/obj/TStnDataBlock.hh"
#include "Stntuple/obj/TCrvRecoPulse.hh"
#include "Stntuple/obj/TCrvCoincidenceCluster.hh"

class TCrvClusterBlock: public TStnDataBlock {
  friend class StntupleInitCrvClusterBlock;

public:
  int            fNPulses = 0;	             // number of reconstructed pulses - only those associated with clusters
  TClonesArray*  fListOfPulses;	             // list of pulses

  int            fNClusters = 0;             // number of reconstructed CrvCoincidenceCluster's
  TClonesArray*  fListOfClusters;            // list of those
  TStnLinkBlock* fClusterPulseLinks;
//-----------------------------------------------------------------------------
//  functions
//-----------------------------------------------------------------------------
public:
					// ****** constructors and destructor
  TCrvClusterBlock();
  virtual ~TCrvClusterBlock();
					// ****** accessors

  Int_t                   NPulses         () { return fNPulses; }
  TCrvRecoPulse*          Pulse      (int i) { return (TCrvRecoPulse*) fListOfPulses->UncheckedAt(i); }
  TClonesArray*           GetListOfPulses () { return fListOfPulses; }

  Int_t                   NClusters        () { return fNClusters; }
  TCrvCoincidenceCluster* Cluster     (int i) { return (i < fListOfClusters->GetEntriesFast()) ? (TCrvCoincidenceCluster*) fListOfClusters->UncheckedAt(i) : nullptr; }
  TClonesArray*           GetListOfClusters() { return fListOfClusters; }


  TStnLinkBlock*          ClusterToPulseLinks    () { return fClusterPulseLinks;     }

  int     NClusterPulses       (int Ic        ) const { return fClusterPulseLinks->NLinks(Ic); }
  int     ClusterPulseIndex    (int Ic, int Ip) const { return fClusterPulseLinks->Index(Ic,Ip); }
//-----------------------------------------------------------------------------
// modifiers
//-----------------------------------------------------------------------------
  TCrvRecoPulse*          NewPulse() { 
    return new ((*fListOfPulses)[fNPulses++]) TCrvRecoPulse();
  }

  TCrvCoincidenceCluster* NewCluster() { 
    return new ((*fListOfClusters)[fNClusters++]) TCrvCoincidenceCluster();
  } 
//-----------------------------------------------------------------------------
// overloaded methods of TObject
//-----------------------------------------------------------------------------
  void Clear(Option_t* opt="");
  void Print(Option_t* opt="") const;

  ClassDef(TCrvClusterBlock,1)	// CRV coincidence cluster block
};


#endif


