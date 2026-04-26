#ifndef STNTUPLE_ALG_PlestidHill_hh
#define STNTUPLE_ALG_PlestidHill_hh
//Michael MacKenzie (2020)

// C++ includes
#include <iostream>
#include <string>
#include <cmath>
#include <memory>
#include <algorithm>

// cetlib includes
// #include "cetlib_except/exception.h"

// CLHEP includes
// #include "CLHEP/Units/PhysicalConstants.h"

//ROOT includes
#include "TF1.h"
#include "TH1.h"
#include "TRandom.h"

class PlestidHillInternalRadiativeCapture {

public :
  int verbose_; // Control output verbosity
  double alpha_;
  double me_;
  double mdecay_;

public :
  PlestidHillInternalRadiativeCapture(bool usePion) : verbose_(1) {
    // GlobalConstantsHandle<ParticleDataTable> pdt;
    me_    = 0.51099895; //pdt->particle(PDGCode::e_minus ).ref().mass().value(); //0.51099895;
    if(usePion)
      mdecay_= 139.57018; //pdt->particle(PDGCode::pi_minus).ref().mass().value(); //139.57018;
    else
      mdecay_= 105.6583755; //pdt->particle(PDGCode::mu_minus).ref().mass().value(); //105.6583755;
    alpha_ = 1./137.036; //CLHEP::fine_structure_const;
  }

  PlestidHillInternalRadiativeCapture() : PlestidHillInternalRadiativeCapture(false) { }

  void Print() {
    std::cout << "PlestidHillInternalRadiativeCapture:" << std::endl
              << " M(e)       = " << me_ << std::endl
              << " M(decay)   = " << mdecay_ << std::endl
              << " alpha      = " << alpha_ << std::endl
              << " verbose    = " << verbose_ << std::endl;
  }

  double Probability(double positron_energy, double photon_energy) {
    //check if physical
    if(positron_energy > photon_energy - me_ || photon_energy < me_ || positron_energy < me_) return 0.;
    //calculate value add/subtracted in m*+-
    double m_val = (positron_energy*positron_energy-me_*me_);
    m_val *= (positron_energy*positron_energy-me_*me_ - 2.*positron_energy*photon_energy + photon_energy*photon_energy);
    m_val = sqrt(m_val);

    //m*+-
    const double m_star_p = sqrt(2.*(me_*me_+m_val+positron_energy*photon_energy-positron_energy*positron_energy));
    const double m_star_m = sqrt(2.*(me_*me_-m_val+positron_energy*photon_energy-positron_energy*positron_energy));


    double prob = alpha_/M_PI/photon_energy*log(m_star_p/m_star_m);
    if(verbose_ > 1)      std::cout << "PlestidHillInternalRadiativeCapture::" << __func__
                                    << ": Probability = "
                                    << prob << " lepton energy = " << positron_energy
                                    << " photon energy = " << photon_energy
                                    << std::endl;

    if(prob < 0.) {
      if(verbose_ > 0)
        std::cout << "PlestidHillInternalRadiativeCapture::" << __func__
                  << ": Negative probability, "
                  << prob << ", for lepton energy = " << positron_energy
                  << " and photon energy = " << photon_energy
                  << "! Setting to 0...\n";
      prob = 0.;
    }
    return prob;
  }

};
#endif
