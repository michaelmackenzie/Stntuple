//-----------------------------------------------------------------------------
//  algorithms working on different STNTUPLE objects
//  Nov 26 2000 P.Murat
//-----------------------------------------------------------------------------

#include <float.h>
#include <iostream>
#include <math.h>
#include "TLorentzVector.h"
#include "TTree.h"
#include "TH1.h"
#include "TString.h"

#include "Stntuple/alg/TStntuple.hh"

#include "Stntuple/loop/TStnModule.hh"

#include "Stntuple/obj/TStnTrack.hh"

#include <Stntuple/obj/TStnHeaderBlock.hh>
#include <Stntuple/obj/TStnTrackBlock.hh>

#include "Stntuple/obj/TStnEvent.hh"
#include "Stntuple/obj/TStnErrorLogger.hh"

#include "Stntuple/alg/TStntuple.hh"

#include "Stntuple/obj/TStnDBManager.hh"


ClassImp(TStntuple)


TStntuple*        TStntuple::fgInstance  = 0;

Int_t             TStntuple::fgRunNumber = 0;

//-----------------------------------------------------------------------------
TStntuple::TStntuple() {

//-----------------------------------------------------------------------------
// initialize the LO DIO spectrum
//-----------------------------------------------------------------------------
  TTree tree("t1","t1");
  double emin = 0.05;
  double emax = 110.05;
  int    nb   = 1100;
  double bin  = 0.1;

  TString table = "Offline/ConditionsService/data/czarnecki_Al.tbl";
  if(true) { //FIXME: Make this configurable
    table = "Offline/ConditionsService/data/heeck_finer_binning_2016_szafron.tbl";
    nb  = 11000; //finer binning in this table
    bin = 0.01;
  }
  tree.ReadFile(table,"e/f:w/f");
  int n = tree.GetEntries();

  fDioSpectrumHist = new TH1D("h_dio_spectrum","DIO spectrum",nb,emin,emax);

  float e, w;

  tree.SetBranchAddress("e",&e);
  tree.SetBranchAddress("w",&w);

  for (int i=0; i<n; i++) {
    tree.GetEntry(i);

    int ibin = (e-emin)/bin+1;
    // skip 0th bin written out (underflows)
    if (ibin > 0) fDioSpectrumHist->SetBinContent(ibin,w);

    //    printf(" %5i %10.3f %12.5e\n",ibin,e,w);
  }

  // Normalize the input spectral shape to per DIO event
  fDioSpectrumHist->Scale(1./(fDioSpectrumHist->Integral() * bin));

  fDioSpectrum = new smooth(&(*fDioSpectrumHist));

  PBar_Striganov_SetP2Max(2.0);
}

//_____________________________________________________________________________
TStntuple::~TStntuple() {
}

//_____________________________________________________________________________
TStntuple*  TStntuple::Instance() { 
  static Cleaner cleaner;
  return (fgInstance) ? fgInstance : (fgInstance = new TStntuple());
}

//------------------------------------------------------------------------------
TStntuple::Cleaner::Cleaner() {
}

//------------------------------------------------------------------------------
TStntuple::Cleaner::~Cleaner() {
  if (TStntuple::fgInstance) {
    delete TStntuple::fgInstance;
    TStntuple::fgInstance = 0;
  }
}

//_____________________________________________________________________________
Int_t TStntuple::Init(Int_t RunNumber) {

  int rc = 0;
//----------------------------------------------------------------------------
// Did we do this alread?
//----------------------------------------------------------------------------
  if (fgRunNumber == RunNumber) return rc;

//----------------------------------------------------------------------------
// Pointer to the data base manager
//----------------------------------------------------------------------------
//  TStnDBManager* dbm = TStnDBManager::Instance();

//----------------------------------------------------------------------------
// Assign the run number to make sure we do not repeat this unnecessarily
//----------------------------------------------------------------------------
  fgRunNumber = RunNumber;

  return rc;
}

//-----------------------------------------------------------------------------
// parameterization of the DIO spectrum on Al
// from Czarnecki et al, Phys.Rev.D84:013006,2011 (http://www.arxiv.org/abs/1106.4756)
// function is normalized to the unit integral,
// full histogram from ConditionsService, so the histogram used has to 
// be divided by the number of events and, then, scaled to the expected number 
// of protons on target
//-----------------------------------------------------------------------------
double TStntuple::DioWeightAlFull(double E) {
  // double w = fDioSpectrum->GetFunc()->Eval(E);
  double w = std::max(0., fDioSpectrumHist->Interpolate(E));
  // double w = fDioSpectrumHist->GetBinContent(fDioSpectrumHist->FindBin(E));
  if(!std::isfinite(w) || w < 0.) {
    const double binval = std::max(0., fDioSpectrumHist->Interpolate(E));
    printf("TStntuple::%s: Error! Undefined spectrum value for DIO with E = %.3f: %f --> Using interpolation of %.3g\n", __func__, E, w, binval);
    w = binval;
  }
  return w;
}

//-----------------------------------------------------------------------------
// parameterization of the DIO spectrum on Al
// from Czarnecki et al, Phys.Rev.D84:013006,2011 (http://www.arxiv.org/abs/1106.4756)
// function is normalized to the unit integral, so the histogram used has to 
// be divided by the number of events and, then, scaled to the expected number 
// of protons on target
//-----------------------------------------------------------------------------
double TStntuple::DioWeightAl(double E) {

  double a5(8.6434), a6(1.16874), a7(-1.87828e-2), a8(9.16327e-3);
  double emu(105.194), mAl(25133.);
  
  double de, de5, w;

  de  = emu-E-E*E/(2*mAl);

  de5 = de*de*de*de*de;

  w   = 1.e-17*de5*(a5 + de*(a6+de*(a7+a8*de)));

  if (de < 0) w = 0;

  return w;
}

//-----------------------------------------------------------------------------
// parameterization of the LO DIO spectrum on Al 
// from mu2e-6309 (by R.Szafron)
//-----------------------------------------------------------------------------
double TStntuple::DioWeightAl_LO(double E) {

  double a5(8.99879), a6(1.17169), a7(-1.06599e-2), a8(8.14251e-3);
  double emu(105.194), mAl(25133.);
  
  double de, de5, w;

  de  = emu-E-E*E/(2*mAl);

  de5 = de*de*de*de*de;

  w   = 1.e-17*de5*(a5 + de*(a6+de*(a7+a8*de)));

  if (de < 0) w = 0;

  return w;
}

//-----------------------------------------------------------------------------
// parameterization of the DIO spectrum on Al with LL radiative corrections 
// from mu2e-6309 (by R.Szafron)
// 'emu' - energy of the muon bound in Al nucleus, not the muon mass
//-----------------------------------------------------------------------------
double TStntuple::DioWeightAl_LL(double E) {

  double a5(8.9), a6(1.17169), a7(-1.06599e-2), a8(8.14251e-3);
  double emu(105.194), mAl(25133.), mmu(105.658), me(0.511);
  double alpha(1./137.036) ;  // alpha EM
  
  double de, de5, w;

  de  = emu-E-E*E/(2*mAl);

  de5 = de*de*de*de*de;

  double f = 2.*log((mmu/me)*(1-de/mmu))-2+2.*log(2);

  w   = 1.e-17*pow(de/mmu,(alpha/M_PI)*f)*de5*(a5 + de*(a6+de*(a7+a8*de)));

  if (de < 0) w = 0;

  return w;
}
//-----------------------------------------------------------------------------
// parameterization of the Michel spectrum with 1st order radiative corrections
// from Landau-Lifshitz('71). Weight is calculated as dw/(MeV)
//-----------------------------------------------------------------------------
double TStntuple::MichelWeight(double E) {

  double a0(0.036594), a1(0.65631), a2(0.84138);
  double mmu(105.658), me(0.511),   Emax=52.8302;
  double alpha(1./137.036) ;  // alpha EM
  double x; x=E/Emax;
  double F, h, L; 
  double PI = 3.14159265359;
  double dw;

  F  = a0+a1*x+a2*x*x;
  L  = log(mmu/me);
  h  = 4*F-(2/3)*(PI*PI)+3*L-4+2*log(x)*(3*log(1-x)-2*log(x)-2*L+1)+2*(2*L-1-1/x)*log(1-x)+6*(1-x)/(3-2*x)*log(x)+(1-x)/(3*x*x*(3-2*x))*((5+17*x-34*x*x)*(L+log(x))-22*x+34*x*x);
  dw = (1+alpha/(2*PI)*h)*(3-2*x)*x*x/Emax;

  dw=dw/0.49791;
  if(dw<=0||x>=0.999999999)
    dw=0;
 
  return dw;
}

//-----------------------------------------------------------------------------
// 
double TStntuple::DioWeightTi(double E) {
  printf(">>> ERROR: TStntuple::DioWeightTi not implemented yet\n");
  return -1;
}

//-----------------------------------------------------------------------------
// RMC closure approximation weight, 
// normalization: integral(weight,0,KMax) = KMax, such that the distribution
// on N events sampled uniformly from 0 to KMax with the given weight 
// has a sum of weights equal to N
//-----------------------------------------------------------------------------
double TStntuple::RMC_ClosureAppxWeight(double K, double KMax) {
  double x, w{20.}, weight{0};

  x = K/KMax;
  if (x < 1)  weight = w*(1-2*x+2*x*x)*(1-x)*(1-x)*x;

  return weight;
  
}

//-----------------------------------------------------------------------------
// RMC plestid phase-space approximation weight
// Normalization is defined to be 1 from 0 - inf dk
// k = photon energy
// kmax = endpoint
// knockout = N(nucleons knocked out)
//-----------------------------------------------------------------------------
double TStntuple::RMC_PlestidWeight(double K, double KMax, int knockout) {
  if(K > KMax || K < 0. || KMax <= 0.) return 0.;
  if(knockout < 0) return 0.;
  const double power = 2. + 1.5*knockout;
  const double norm = (power + 1.) * (power + 2.) / KMax;
  const double x = K / KMax;
  const double weight = norm * x * std::pow(1. - x, power);
  return weight;
}

// Integral from k_1 to k_2 of the Plestid phase-space approximation spectrum shape
double TStntuple::RMC_PlestidIntegral(double K_1, double K_2, double KMax, int knockout) {
  if(KMax <= 0.) return 0.;
  if(knockout < 0) return 0.;
  K_1 = std::max(0., std::min(KMax, K_1));
  K_2 = std::max(0., std::min(KMax, K_2));
  if(K_1 >= K_2) return 0.;
  const double power = 2. + 1.5*knockout;
  const double x_1 = K_1 / KMax;
  const double x_2 = K_2 / KMax;
  const double val_1 = (x_1 - 1.)*std::pow(1-x_1, power)*(power*x_1+x_1+1.);
  const double val_2 = (x_2 - 1.)*std::pow(1-x_2, power)*(power*x_2+x_2+1.);
  const double integral = val_2 - val_1;
  return integral;
}

//-----------------------------------------------------------------------------
// RPC photon weight 
// normalization: integral(weight,0,eMax) = eMax, such that the distribution
// on N events sampled uniformly from 0 to eMax with the given weight has an 
// integral of N
// unlike the RMC closure approximation spectrum, this is simply a parameterization 
// of the experimental data (Bistirlich et al), hence only one parameter 
//-----------------------------------------------------------------------------
double TStntuple::RPC_PhotonEnergyWeight(double E) {

  double eMax(134.530), alpha(1.29931), gamma(0.928705), tau(9.39676), c0(4.77611e-02),c1(-3.26349e-04);
  double fint(0.9845620786721507);

  double f(0.);

  if (E < eMax) f = eMax*pow(eMax-E,alpha)*exp(-(eMax-gamma*E)/tau)*(c0+c1*E)/fint;
  return f;
}

//-----------------------------------------------------------------------------
// Giant dipole resonance mu- --> e+ weight
// Normalization does not correct for cutoffs due to requiring physical values
//-----------------------------------------------------------------------------
double TStntuple::MumEp_GDR_Weight(double energy, double emax) {
  if(energy > emax || energy < 0.) return 0.;
  double gamma(6.1), m0(21.1), eloss(emax-energy);
  double mdiff_2 = (eloss - m0) * (eloss - m0);
  double weight = 1./M_PI * (gamma/2.) / (mdiff_2 + (gamma/2.)*(gamma/2.));
  return weight;
}
