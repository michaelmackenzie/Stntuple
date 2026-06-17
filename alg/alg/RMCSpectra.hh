#ifndef STNTUPLE_ALG_RMCSpectra_hh
#define STNTUPLE_ALG_RMCSpectra_hh

//Michael MacKenzie (2020)

// C++ includes
#include <iostream>
#include <string>
#include <cmath>
#include <memory>
#include <algorithm>


//ROOT includes
#include "TF1.h"
#include "TH1.h"
#include "TRandom3.h"

//Mu2e includes
#include "Stntuple/alg/PlestidHillInternalRadiativeCapture.hh"
#include "Stntuple/alg/KrollWadaJosephInternalRadiativeCapture.hh"

class RMCSpectra {

public :
  double kmax_cl_; // Closure approximation endpoint
  double kmax_kn_; // Kinematic endpoint
  int external_version_; // Which external conversion spectrum to use
  int internal_;   // Whether doing internal conversion or not
  int internal_version_; // Which internal conversion spectrum to use
  bool norm_pl_hill_; //whether to force unit norm or allow natural normalization
  int ngen_; //number of random samples when convolving spectra
  TF1 *fSpectrum_; // Spectrum for analytic spectra
  TH1D *hSpectrum_; // Spectrum from convolving spectra
  int seed_; // Random number generator seed
  TRandom3 *rand_; // Random number generator for convolving spectra
  int verbose_; // Control output verbosity
  PlestidHillInternalRadiativeCapture pl_hill_int_; // Internal conversion spectrum
  KrollWadaJosephInternalRadiativeCapture kwj_int_; // Internal conversion spectrum
  enum {kSpectrumVar = 10};
  float var_[kSpectrumVar]; //spectrum variables, if needed
  //external spectra
  enum {kClosure, kFlat, kModifiedClosure, kClosureFlat, kDeltaLine, kTwoClosures, kPlestid};
public :
  RMCSpectra() : kmax_cl_(90.1), kmax_kn_(101.866),
                 external_version_(kClosure), internal_(0),
                 internal_version_(0), norm_pl_hill_(false),
                 ngen_(1e8),
                 hSpectrum_(0), seed_(90),
                 rand_(new TRandom3(seed_)),
                 verbose_(1) {
    for(int i = 0; i < kSpectrumVar; ++i) var_[i] = 0.;
  }
  RMCSpectra(double kmax) : RMCSpectra() {
    kmax_cl_=kmax;
  }

  RMCSpectra(double kmax, double kmax_end, int ext_version) : RMCSpectra(kmax) {
    kmax_kn_=kmax_end;
    external_version_=ext_version;
    if(external_version_ == kModifiedClosure) SetVar(0, 5.10); //modified closure power law tail
    if(external_version_ == kClosureFlat) SetVar(0, 3./2751.); //modified closure with flat tail
    if(external_version_ == kTwoClosures) { //add two closure approximations
      SetVar(0, kmax_end);
      SetVar(1, 0.018); //1.8% kinematic endpoint contribution, or 1 event above 90 MeV per 1000 events above 57 MeV for k1 = 90.1 k2 = 101.866
      //for kmax = 90.1, 14.8% of the spectrum is above 57 MeV
      //for kmax = 101.866, 0.810% of the spectrum is above 90 MeV
      //For 3,000 photons above 57 MeV assume an expectation of 3 photons above 90 MeV from kmax_2 = 101.866
      //So 3 = (N) * 0.810 * 3,000 / 14.8 --> N = 0.018
    }
  }

  RMCSpectra(double kmax, double kmax_end, int ext_version, int internal, int int_version) : RMCSpectra(kmax,kmax_end,ext_version) {
    internal_=internal;
    internal_version_=int_version;
  }

  double Weight(double energy);

  bool SetVar(int index, double val) {
    if(index >= kSpectrumVar) return false;
    var_[index] = val;
    return true;
  }

  void NormalizePlestidHill(bool norm = true) {norm_pl_hill_ = norm;}
  void Print() {
    std::cout << "RMCSpectra: " << std::endl
              << " kmax (closure)         = " << kmax_cl_ << std::endl
              << " kmax (endpoint)        = " << kmax_kn_ << std::endl
              << " external_version       = " << external_version_ << std::endl
              << " internal               = " << internal_ << std::endl
              << " internal_version       = " << internal_version_ << std::endl
              << " normalize plestid_hill = " << norm_pl_hill_ << std::endl
              << " N(samples) to convolve = " << ngen_ << std::endl
              << " verbose                = " << verbose_ << std::endl;
    if(external_version_ == kModifiedClosure) {
      std::cout << " alpha                  = " << var_[0] << std::endl;
    } else if(external_version_ == kClosureFlat) {
      std::cout << " br(flat)               = " << var_[0] << std::endl;
    } else if(external_version_ == kTwoClosures) {
      std::cout << " kmax(2)                = " << var_[0] << std::endl;
      std::cout << " Br(kmax(2))/Br(kmax)   = " << var_[1] << std::endl;
    }
    if(verbose_ > 2) std::cout << "RMCSpectra versions:" << std::endl
                               << " External:" << std::endl
                               << "  -1: Flat" << std::endl
                               << "   0: Closure Approximation" << std::endl
                               << "   2: Modified Closure Approximation" << std::endl
                               << "   3: Closure Approximation + flat" << std::endl
                               << "   4: Kinematic limit delta line" << std::endl
                               << "   5: Two closure approximations" << std::endl
                               << " Internal:" << std::endl
                               << "   0: Kroll+Wada with rho dependence on E removed (Offline Default)" << std::endl
                               << "   1: Kroll+Wada with RPC on hydrogen parameters ignored" << std::endl
                               << "   2: Plestid+Hill" << std::endl;
  }

  void InitializeSpectrum();

private :
  void ConvolveInternal();
  void InitializePlestidHillHistogram();
  TH1D* GetRhoVsEHist(int entries);
  void InitializeKrollWadaHistogram(bool removeRho);
};
#endif
