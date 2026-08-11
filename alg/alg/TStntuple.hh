#ifndef STNTUPLE_ALG_TStntuple_hh
#define STNTUPLE_ALG_TStntuple_hh

#include <math.h>
#include "TClonesArray.h"
#include "TVector2.h"
#include "TVector3.h"
#include "Stntuple/alg/smooth.hh"

class TStnTrack;
class TStnCluster;
class TStnElectron;

class TStntuple: public TObject {
protected:
  static TStntuple*  fgInstance;

  static Int_t       fgRunNumber;
  static Float_t     fgEventVertex;

  TH1D*              fDioSpectrumHist;
  smooth*            fDioSpectrum;

  class  Cleaner {
  public:
    Cleaner ();
    ~Cleaner();
  };
  friend class Cleaner;
//-----------------------------------------------------------------------------
//  methods
//-----------------------------------------------------------------------------
public:
                                // don't use the constructor! use Instance()
                                // instead (if ever needed...)
  TStntuple();
  virtual ~TStntuple();

  static TStntuple*  Instance();

  static Int_t     Init(Int_t RunNumber);
//-----------------------------------------------------------------------------
// full DIO spectrum on Al from 
// Czarnecki et al, Phys.Rev.D84:013006,2011 (http://www.arxiv.org/abs/1106.4756)
// interpolation of a histogram tabulated in EventGenerator/data/czarnecki_Al.tbl
// more details in mu2e-3281
//-----------------------------------------------------------------------------
  double DioWeightAlFull(double P);
  TH1D*  DioSpectrumHist() { return fDioSpectrumHist; }
//-----------------------------------------------------------------------------
// polynomial parameterization of the DIO spectrum on Al from 
// Czarnecki et al, Phys.Rev.D84:013006,2011 (http://www.arxiv.org/abs/1106.4756)
// good from Emax down to about 85 MeV
//-----------------------------------------------------------------------------
  static double DioWeightAl   (double P);

//-----------------------------------------------------------------------------
// parameterization of the LO and LL DIO spectra on Al from 
// from mu2e-6309 (by R.Szafron)
//-----------------------------------------------------------------------------
  static double DioWeightAl_LO(double P);
  static double DioWeightAl_LL(double P);
  static double MichelWeight  (double E);
  static double DioWeightTi   (double P);

  static double RMC_ClosureAppxWeight  (double K, double KMax);
  static double RMC_ClosureAppxIntegral(double K_1, double K_2, double KMax);
  static double RMC_PlestidWeight      (double K, double KMax, int knockout);
  static double RMC_PlestidIntegral    (double K_1, double K_2, double KMax, int knockout);
  static double RPC_PhotonEnergyWeight (double E);


//-----------------------------------------------------------------------------
// parameterization of the LO giant dipole resonance (GDR) for mu- --> e+ on Al
// from mu2e-21887 based on Phys Lett B746(2017)157
//-----------------------------------------------------------------------------
  static double MumEp_GDR_Weight(double energy, double emax);

//-----------------------------------------------------------------------------
// parameterization of inclusive antiproton production cross-section by S.Striganov (mu2e-1776)
// PBeam - proton beam momentum
// PLab, ThLab - momentum and production angle of the outgoing antiproton
//-----------------------------------------------------------------------------
  static double PBar_Striganov_Ed3SigdP3 (double PBeam, double PLab, double ThLab);
  static double PBar_Striganov_d2N       (double PBeam, double PLab, double ThLab); // d^2Sigma/dP/dcosth (lab)
  static void   PBar_Striganov_SetP2Max  (double P2Max);
//-----------------------------------------------------------------------------
// print routines - sometimes it is not possible to do it from a single block
//-----------------------------------------------------------------------------
  // static int  PrintElectron(TStnElectron*       Ele,
  // 			    TStnElectronBlock*  ElectronBlock,
  // 			    TCalDataBlock*      CalDataBlock);
  ClassDef(TStntuple,0)
};

#endif

