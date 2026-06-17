#include "Stntuple/alg/RMCSpectra.hh"
#include "Stntuple/alg/TStntuple.hh"

//Evaluate the spectrum for a given photon/daughter energy
double RMCSpectra::Weight(double energy) {
  double wt(1.);
  if(internal_ == 0) {
    if(!fSpectrum_) {
      std::cout << "RMCSpectra::" << __func__ << ": Spectrum not initialized!\n";
      return wt;
    }
    wt = fSpectrum_->Eval(energy); //external uses function spectrum
  } else if(internal_ >  0) {
    if(!hSpectrum_) {
      std::cout << "RMCSpectra::" << __func__ << ": Spectrum not initialized!\n";
      return wt;
    }
    wt = hSpectrum_->Interpolate(energy); //internal uses histogram spectrum
  }
  if(verbose_ > 1) std::cout << "RMCSpectra::" << __func__ << ": E = " << energy
                             << " wt = " << wt << std::endl;
  return wt;
}

void RMCSpectra::InitializeSpectrum() {
  if(external_version_ == kClosure) { //closure approximation
    fSpectrum_ = new TF1("closure", "(x < [0]) * 20./[0]*(1-2*x/[0]+2*(x/[0])^2)*x/[0]*(1-x/[0])^2", 0., kmax_kn_);
    fSpectrum_->FixParameter(0, kmax_cl_);
  } else if(external_version_ == kFlat) { //flat spectrum
    fSpectrum_ = new TF1("flat", "1./([0] - 1.022)", 0., kmax_kn_);
    fSpectrum_->FixParameter(0, kmax_kn_);
  } else if(external_version_ == kModifiedClosure) { //closure approximation + power law tail
    TF1* ftmp = new TF1("closure", "(x < [0])*20./[0]*(1-2*(x/[0])+2*((x/[0])^2))*(x/[0])*((1-x/[0])^ 2 )", 0., kmax_cl_);
    fSpectrum_ = new TF1("mod_closure",  "[2]*20./[0]*(1-2*(x/[0])+2*((x/[0])^2))*(x/[0])*((1-x/[0])^[1])", 0., kmax_kn_);
    ftmp->FixParameter(0., kmax_cl_);
    fSpectrum_->SetParameters(kmax_kn_, var_[0], 1.);
    fSpectrum_->SetParameter(2, ftmp->Integral(57., kmax_cl_)/fSpectrum_->Integral(57., kmax_cl_)); //match closure integral in measured region
  } else if(external_version_ == kClosureFlat) { //closure approximation + flat tail
    TF1* ftmp = new TF1("closure"     , "(x < [0])*20./[0]*(1-2*(x/[0])+2*((x/[0])^2))*(x/[0])*((1-x/[0])^ 2)", 0., kmax_cl_);
    fSpectrum_ = new TF1("mod_closure", "(x < [0])*20./[0]*(1-2*(x/[0])+2*((x/[0])^2))*(x/[0])*((1-x/[0])^ 2) + [1]", 0., kmax_kn_);
    ftmp->FixParameter(0., kmax_cl_);
    fSpectrum_->SetParameters(kmax_cl_, 1.);
    float br_tail = var_[0];
    fSpectrum_->SetParameter(1, br_tail*ftmp->Integral(57., kmax_cl_)/fSpectrum_->Integral(kmax_cl_, kmax_kn_)); //set branching ratio above 90
  } else if(external_version_ == kDeltaLine) { //only allow kinematic endpoint
    fSpectrum_ = new TF1("endpoint", "1.", kmax_kn_-0.01, kmax_kn_+0.01);
  } else if(external_version_ == kTwoClosures) {
    TString formula = "(x < [0])*20./[0]*(1-2*(x/[0])+2*((x/[0])^2))*(x/[0])*((1-x/[0])^ 2)"; //nominal closure
    formula += " + (x < [1])*[2]*20./[1]*(1-2*(x/[1])+2*((x/[1])^2))*(x/[1])*((1-x/[1])^ 2)"; //second, reduced closure
    fSpectrum_ = new TF1("two_closure", formula.Data(), 0., kmax_kn_);
    float k_two = var_[0];
    float br_two = var_[1];
    fSpectrum_->SetParameters(kmax_cl_, k_two, br_two);
  } else if(external_version_ == kPlestid) {
    constexpr double br_0n_ref     = 0.099; // defined for E_photon > 57 MeV
    constexpr double br_1n_ref     = 0.901;
    constexpr double kmax_0n       = 101.866;
    constexpr double kmax_1n       =  95.449;
    constexpr double br_ref_energy = 57.; // Get the branching fractions for the full space
    const     double r_0n_full     = (br_0n_ref <= 0.) ? 0. : br_0n_ref / TStntuple::RMC_PlestidIntegral(br_ref_energy, kmax_0n, kmax_0n, 0);
    const     double r_1n_full     = (br_1n_ref <= 0.) ? 0. : br_1n_ref / TStntuple::RMC_PlestidIntegral(br_ref_energy, kmax_1n, kmax_1n, 1);

    // Set to normalized above 57 MeV
    double integral = 0.;
    integral += r_0n_full * TStntuple::RMC_PlestidIntegral(br_ref_energy, kmax_0n, kmax_0n, 0);
    integral += r_1n_full * TStntuple::RMC_PlestidIntegral(br_ref_energy, kmax_1n, kmax_1n, 1);
    const     double br_0n         = r_0n_full / integral;
    const     double br_1n         = r_1n_full / integral;

    // Define the two endpoint function
    TString function =    "(x < [0]) * [1] * [2] * (x/[0] * (max(0., 1 - x/[0]))^[3])"; // 0 knockout
    function        += " + (x < [4]) * [5] * [6] * (x/[4] * (max(0., 1 - x/[4]))^[7])"; // 1 knockout
    fSpectrum_ = new TF1("plestid", function.Data(), 0., kmax_0n);
    const double power_0 = 2.;
    const double power_1 = 3.5;
    fSpectrum_->SetParameters(kmax_0n, br_0n, (power_0 + 1.) * (power_0 + 2.) / kmax_0n, power_0,
                              kmax_1n, br_1n, (power_1 + 1.) * (power_1 + 2.) / kmax_1n, power_1);
  } else {
    std::cout << "UNKNOWN External RMC Version " << external_version_ << "! Exiting...\n";
    return;
  }
  if(internal_)
    //convolve external photon spectrum with internal conversion to get e+/e-
    ConvolveInternal();
  else if(verbose_ > 0) Print();
}

void RMCSpectra::ConvolveInternal() {
  if(verbose_ > 0) Print();
  if(verbose_ > 0) std::cout << "RMCSpectra: Beginning internal spectrum convolution!\n";
  if(internal_version_ == 0 || internal_version_ == 1) { //Kroll+Wada+Joseph spectrum
    kwj_int_.use_proton_values_ = (internal_version_ == 0); //Offline version uses values from capture on proton
    if(verbose_ > 0) kwj_int_.Print();
    //Current Mu2e Offline implementation assumes rho is not a function of photon energy --> remove dependence
    //fixed rho(E_gamma) := rho if version = 0
    InitializeKrollWadaHistogram(internal_version_ == 0);
  } else if(internal_version_ == 2) { //Plestid+Hill spectrum
    if(verbose_ > 0) pl_hill_int_.Print();
    InitializePlestidHillHistogram();
  }
}

void RMCSpectra::InitializePlestidHillHistogram() {
  const int nentries = ngen_; //entries to make convolution

  hSpectrum_ = new TH1D("int_conv", "int_conv", 1000, 0., (external_version_ == kClosure) ? kmax_cl_ : kmax_kn_);
  for(int entry = 0; entry < nentries; ++entry) {
    double photon_energy(0.), positron_energy(1.), prob(1.);
    //get photon energy
    if(external_version_ == kClosure) { //closure approximation
      photon_energy = 2.*pl_hill_int_.me_ + (kmax_cl_ - 2.*pl_hill_int_.me_)*rand_->Uniform(); //needs at least 2*electron mass
      prob = fSpectrum_->Eval(photon_energy) * (kmax_cl_ - 2.*pl_hill_int_.me_); //external PDF / (flat PDF)
    } else if(external_version_ == kDeltaLine) { //kinematic endpoint
      photon_energy = kmax_kn_;
      prob = 1.;
    } else { //closure approximation + some sort of tail
      photon_energy = 2.*pl_hill_int_.me_ + (kmax_kn_ - 2.*pl_hill_int_.me_)*rand_->Uniform(); //needs at least 2*electron mass
      prob = fSpectrum_->Eval(photon_energy) * (kmax_kn_ - 2.*pl_hill_int_.me_); //external PDF / (flat PDF)
    }

    //get internal conversion given photon energy
    if(internal_version_ == 2) {
      positron_energy = pl_hill_int_.me_ + (photon_energy - 2.*pl_hill_int_.me_)*rand_->Uniform(); //needs at least 1 mass, and leave 1 mass
      prob *= pl_hill_int_.Probability(positron_energy, photon_energy) * (photon_energy - 2.*pl_hill_int_.me_); //internal PDF / (flat PDF)
    }
    prob /= nentries*hSpectrum_->GetBinWidth(1);
    hSpectrum_->Fill(positron_energy, prob);
  }
  if(norm_pl_hill_) hSpectrum_->Scale(1./hSpectrum_->Integral()/hSpectrum_->GetBinWidth(1)); //Force unit norm
  if(verbose_ > 0) printf("RMCSpectra::%s: Spectrum integral*BinWidth = %.3e\n", __func__, hSpectrum_->Integral()*hSpectrum_->GetBinWidth(1));
}

//Get the branching fraction of Kroll+Wada as a function of photon energy
TH1D* RMCSpectra::GetRhoVsEHist(int entries) {
  const double me = kwj_int_.me_;
  double emax = (external_version_ == kClosure) ? kmax_cl_ : kmax_kn_;
  double emin = (external_version_ == kDeltaLine) ? kmax_kn_-0.01 : 2.*me;
  TH1D* h = new TH1D("hRhoVsE", "hRhoVsE", 1000, 0., emax);
  if(verbose_ > 0) std::cout << " initializing Kroll+Wada Rho vs Energy histogram\n";
  for(int i = 0; i < entries; ++i) {
    const double e_g = emin + (emax-emin)*rand_->Uniform();
    const double x   = 2.*me + (e_g-2.*me)*rand_->Uniform();
    const double mxy = sqrt(1.-(2.*me/x)*(2.*me/x));
    const double y   = 2.*mxy*(rand_->Uniform()-0.5);

    double kw  = kwj_int_.Probability(x,y,e_g)*2.*mxy*(e_g-2.*me)*(emax-2.*me);
    if(kw <= 0) {
      if(verbose_ > 1) std::cout << "RMCSpectra::" << __func__ << ": < 0 internal probability! e_g = "
                                 << e_g << " x = " << x << " y = " << y << " mxy = " << mxy << std::endl;
      kw = 0.;
    }

    h->Fill(e_g, kw/entries);
  }
  return h;
}

//returns a normalized histogram to get the probability distribution
//of producing an internal electron/positron of energy E given kMax
//using closure approximation + Kroll+Wada
void RMCSpectra::InitializeKrollWadaHistogram(bool removeRho) {
  if(verbose_ > 0) std::cout << " Using removeRho = " << removeRho << std::endl;
  kwj_int_.verbose_ = (verbose_ > 0) ? verbose_ - 1 : 0;
  int entries = ngen_;
  const double me = kwj_int_.me_;
  double emax = (external_version_ == kClosure) ? kmax_cl_ : kmax_kn_;
  double emin = (external_version_ == kDeltaLine) ? kmax_kn_ - 0.02 : 2.*me;
  hSpectrum_ = new TH1D("int_conv", "int_conv", 1000, 0., emax);
  TH1D* hRhoVsEg = (removeRho && external_version_ != kDeltaLine) ? GetRhoVsEHist(2*entries) : 0;
  TH1D* h = (verbose_ > 1) ? new TH1D("hRhoVsEDebug", "hRhoVsEDebug", 1000, 0., emax) : 0; //for debugging
  TH1D* h2 = (verbose_ > 1) ? new TH1D("hExt", "hExt", 1000, 0., emax) : 0; //for debugging
  if(verbose_ > 0) std::cout << " initializing Kroll+Wada Energy histogram\n";
  for(int i = 0; i < entries; ++i) {
    //photon energy
    const double e_g = emin + (emax-emin)*rand_->Uniform();
    //pair parameters
    const double x   = 2.*me + (e_g-2.*me)*rand_->Uniform();
    const double mxy = sqrt(1.-(2.*me/x)*(2.*me/x));//maximum y
    const double y   = 2.*mxy*(rand_->Uniform()-0.5);

    //internal conversion weight
    double kw  = kwj_int_.Probability(x,y,e_g)*(2.*mxy)*(e_g-2.*me);
    if(kw <= 0) {
      if(verbose_ > 1) std::cout << "RMCSpectra::" << __func__ << ": < 0 internal probability! e_g = "
                                 << e_g << " x = " << x << " y = " << y << " mxy = " << mxy << std::endl;
      kw = 0.;
    }

    //real photon weight
    const double cl  = (external_version_ == kDeltaLine) ? 1. : fSpectrum_->Eval(e_g)*(emax-emin);
    double prob = cl*kw;

    double energy = 0.;
    if(prob <= 0.) {
      if(verbose_ > 1) std::cout << "RMCSpectra::" << __func__ << ": < 0 probability! e_g = "
                                 << e_g << " x = " << x << " y = " << y << " mxy = " << mxy << std::endl;
      prob = 0.;
    }
    else {
      if(hRhoVsEg)
        prob /= hRhoVsEg->Interpolate(e_g);
      //get a daughter energy from parameters
      energy = 0.5*(e_g + y*sqrt(e_g*e_g - x*x));
    }
    prob /= entries*hSpectrum_->GetBinWidth(1);
    hSpectrum_->Fill(energy, prob);
    if(h)  h ->Fill(e_g, prob/cl*(emax-2.*me));
    if(h2) h2->Fill(e_g, cl/entries/h2->GetBinWidth(1));
  }
  if(verbose_ > 1) printf("RMCSpectra::%s: Spectrum integral before normalization = %.3e\n", __func__, hSpectrum_->Integral()*hSpectrum_->GetBinWidth(1));
  if(internal_version_ == 0) hSpectrum_->Scale(1./hSpectrum_->Integral()/hSpectrum_->GetBinWidth(1)); //Force unit norm in Mu2e default use
  if(!h)
    delete hRhoVsEg;
  else {
    if(hRhoVsEg) hRhoVsEg->Draw();
    h->SetLineColor(kRed); h->Draw((internal_version_ == 0) ? "sames" : "");
    h2->SetLineColor(kGreen); h2->Draw("sames");
    fSpectrum_->Draw("sames");
    hSpectrum_->SetLineColor(kYellow); hSpectrum_->Draw("sames");
  }
}
