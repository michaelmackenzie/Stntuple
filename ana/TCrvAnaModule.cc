//////////////////////////////////////////////////////////////////////////////
// use of tmp:
//
// use of debug bits: bits 0-2 are reserved
//  0  : all events
//  1  : passed events
//  2  : rejected events
//  3  : N(CalHelixFinder helices hel>0) > 0
//  4  : N(CalHelixFinder helices hel<0) > 0
// 
///////////////////////////////////////////////////////////////////////////////
#include "TF1.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TEnv.h"
#include "TSystem.h"

#include "Stntuple/loop/TStnAna.hh"
#include "Stntuple/obj/TStnHeaderBlock.hh"
#include "Stntuple/alg/TStntuple.hh"
#include "Stntuple/geom/TDisk.hh"
#include "Stntuple/val/stntuple_val_functions.hh"
//------------------------------------------------------------------------------
// Mu2e offline includes
//-----------------------------------------------------------------------------
#include "ana/TCrvAnaModule.hh"


ClassImp(stntuple::TCrvAnaModule)

namespace stntuple {
//-----------------------------------------------------------------------------
TCrvAnaModule::TCrvAnaModule(const char* name, const char* title):
  TStnModule(name,title)
{
  fPtMin  = 1.;

  fMinT0 = 0; // do not cut on time by default
//-----------------------------------------------------------------------------
// MC truth: define which MC particle to consider as signal
//-----------------------------------------------------------------------------
  fPdgCode     = 11;
  fProcessCode = 2;			// conversionGun, 28:StoppedParticleReactionGun

  // fHelixBlockName[0] = "HelixBlockTprDe";
  // fHelixBlockName[1] = "HelixBlockCprDe";
  // fHelixBlockName[2] = "HelixBlockDe";
}

//-----------------------------------------------------------------------------
TCrvAnaModule::~TCrvAnaModule() {
}


//-----------------------------------------------------------------------------
void TCrvAnaModule::BookCrvClusterHistograms   (CrvClusterHist_t*   Hist, const char* Folder){
 
  HBook1F(Hist->fSectorType    ,"sector"     ,Form("%s: CRV sector type" ,Folder), 150,   0,   150,Folder);
  HBook1F(Hist->fNPulses       ,"npulses"    ,Form("%s: N(pulses)"       ,Folder), 100,   0,   100,Folder);
  HBook1F(Hist->fNPe           ,"npe"        ,Form("%s: N(PE)"           ,Folder), 500,   0,  5000,Folder);
  HBook1F(Hist->fStartTime     ,"tstart"     ,Form("%s: start time, ns"  ,Folder), 400,   0,  2000,Folder);  
  HBook1F(Hist->fEndTime       ,"tend"       ,Form("%s: end time, ns"    ,Folder), 400,   0,  2000,Folder);  
  HBook1F(Hist->fWidth         ,"wwidth"     ,Form("%s: width, ns"       ,Folder), 200,   0,  200,Folder);  
  HBook2F(Hist->fXVsZ          ,"x_vs_z"     ,Form("%s: X vs Z"          ,Folder), 250,   -5000,20000,200,-10000,10000,Folder);  
  HBook2F(Hist->fYVsZ          ,"y_vs_z"     ,Form("%s: Y vs Z"          ,Folder), 250,   -5000,20000,200,     0,4000,Folder);  
}

//-----------------------------------------------------------------------------
void TCrvAnaModule::BookCrvPulseHistograms   (CrvPulseHist_t*   Hist, const char* Folder){

  HBook1F(Hist->fNPe       ,"npe"        ,Form("%s: N(Pe)"     ,Folder), 250,   0,   500,Folder);
  HBook1F(Hist->fNPeHeight ,"npe_height" ,Form("%s: NPE_HEIGHT",Folder), 250,   0,   500,Folder);
  HBook1F(Hist->fNDigis    ,"ndigis"     ,Form("%s: N(digis)"  ,Folder),  10,   0,    10,Folder);
  HBook1F(Hist->fBar       ,"bar"        ,Form("%s: bar"       ,Folder), 600,   0,  6000,Folder);
  HBook1F(Hist->fSipm      ,"sipm"       ,Form("%s: sipm"      ,Folder),  10,   0,    10,Folder);

  HBook1F(Hist->fTime      ,"time"       ,Form("%s: time"      ,Folder), 500,   0,  2000,Folder);
  HBook1F(Hist->fHeight    ,"height"     ,Form("%s: height"    ,Folder), 500,   0,  2000,Folder);
  HBook1F(Hist->fWidth     ,"width"      ,Form("%s: width"     ,Folder), 500,   0,   500,Folder);
  HBook1F(Hist->fChi2      ,"chi2"       ,Form("%s: chi2"      ,Folder), 500,   0,   500,Folder);
  HBook1F(Hist->fLeTime    ,"le_time"    ,Form("%s: LE time"   ,Folder), 500,   0,  2000,Folder);
  HBook1F(Hist->fDt        ,"dt"         ,Form("%s: LEt-t"     ,Folder), 100, -50,    50,Folder);
}

//-----------------------------------------------------------------------------
void TCrvAnaModule::BookGenpHistograms(GenpHist_t* Hist, const char* Folder) {
//   char name [200];
//   char title[200];

  HBook1F(Hist->fP      ,"p"       ,Form("%s: Momentum"     ,Folder),1000,     0, 200,Folder);
  HBook1F(Hist->fPdgCode[0],"pdg_code_0",Form("%s: PDG Code[0]"     ,Folder),200, -100, 100,Folder);
  HBook1F(Hist->fPdgCode[1],"pdg_code_1",Form("%s: PDG Code[1]"     ,Folder),500, -2500, 2500,Folder);
  HBook1F(Hist->fGenID  ,"gen_id"  ,Form("%s: Generator ID" ,Folder), 100,     0, 100,Folder);
  HBook1F(Hist->fZ0     ,"z0"      ,Form("%s: Z0"           ,Folder), 500,  5400, 6400,Folder);
  HBook1F(Hist->fT0     ,"t0"      ,Form("%s: T0"           ,Folder), 200,     0, 2000,Folder);
  HBook1F(Hist->fR0     ,"r"       ,Form("%s: R0"           ,Folder), 100,     0,  100,Folder);
  HBook1F(Hist->fCosTh  ,"cos_th"  ,Form("%s: Cos(Theta)"   ,Folder), 200,   -1.,   1.,Folder);
}


//-----------------------------------------------------------------------------
void TCrvAnaModule::BookEventHistograms(EventHist_t* Hist, const char* Folder) {
  //  char name [200];
  //  char title[200];

  //  HBook1F(Hist->fEleCosTh    ,"ce_costh",Form("%s: Conversion Electron Cos(Theta)"  ,Folder),100,-1,1,Folder);
  //  HBook1F(Hist->fEleMom      ,"ce_mom"  ,Form("%s: Conversion Electron Momentum"    ,Folder),1000,  0,200,Folder);
  HBook1F(Hist->fRv              ,"rv"      ,Form("%s: R(Vertex)"                       ,Folder), 100, 0, 1000,Folder);
  HBook1F(Hist->fZv              ,"zv"      ,Form("%s: Z(Vertex)"                       ,Folder), 300, 0,15000,Folder);
  HBook1F(Hist->fNGenp           ,"ngenp"   ,Form("%s: N(Gen Particles)"                ,Folder),500,0,500,Folder);
  HBook1F(Hist->fNCrvClusters    ,"ncrvcl"  ,Form("%s: N(CRV Clusters)"                 ,Folder),100,0, 100,Folder);
  HBook1F(Hist->fNCrvCoincidences,"ncrvco"  ,Form("%s: N(CRV coincidences)"             ,Folder),200,0,1000,Folder);
  HBook1F(Hist->fNCrvPulses      ,"ncrvp"   ,Form("%s: N(CRV pulses)"                   ,Folder),500,0,10000,Folder);
}

//-----------------------------------------------------------------------------
void TCrvAnaModule::BookSimpHistograms(SimpHist_t* Hist, const char* Folder) {
  //  char name [200];
  //  char title[200];

  HBook1F(Hist->fPdgCode   ,"pdg"         ,Form("%s: PDG code"                     ,Folder),200,-100,100,Folder);
  //  HBook1F(Hist->fNStrawHits,"nsth"        ,Form("%s: n straw hits"                 ,Folder),200,   0,200,Folder);
  HBook1F(Hist->fMomTargetEnd    ,"ptarg" ,Form("%s: CE mom after Stopping Target" ,Folder),400,  90,110,Folder);
  HBook1F(Hist->fMomTrackerFront ,"pfront",Form("%s: CE mom at the Tracker Front"  ,Folder),400,  90,110,Folder);
}


//_____________________________________________________________________________
void TCrvAnaModule::BookHistograms() {

  //  char name [200];
  //  char title[200];
  
  TFolder* fol;
  TFolder* hist_folder;
  char     folder_name[200];
  
  DeleteHistograms();
  hist_folder = (TFolder*) GetFolder()->FindObject("Hist");
//-----------------------------------------------------------------------------
// book event histograms
//-----------------------------------------------------------------------------
  int book_event_histset[kNEventHistSets];
  for (int i=0; i<kNEventHistSets; i++) book_event_histset[i] = 0;

  book_event_histset[ 0] = 1;		// all events
  book_event_histset[ 1] = 1;	        // events with N>0 reconstructed helices
  book_event_histset[ 2] = 1;	        // events with N=0 reconstructed helices

  for (int i=0; i<kNEventHistSets; i++) {
    if (book_event_histset[i] != 0) {
      sprintf(folder_name,"evt_%i",i);
      fol = (TFolder*) hist_folder->FindObject(folder_name);
      if (! fol) fol = hist_folder->AddFolder(folder_name,folder_name);
      fHist.fEvent[i] = new EventHist_t;
      BookEventHistograms(fHist.fEvent[i],Form("Hist/%s",folder_name));
    }
  }
//-----------------------------------------------------------------------------
// book simp histograms
//-----------------------------------------------------------------------------
  int book_simp_histset[kNSimpHistSets];
  for (int i=0; i<kNSimpHistSets; i++) book_simp_histset[i] = 0;

  book_simp_histset[ 0] = 1;		// all events

  for (int i=0; i<kNSimpHistSets; i++) {
    if (book_simp_histset[i] != 0) {
      sprintf(folder_name,"sim_%i",i);
      fol = (TFolder*) hist_folder->FindObject(folder_name);
      if (! fol) fol = hist_folder->AddFolder(folder_name,folder_name);
      fHist.fSimp[i] = new SimpHist_t;
      BookSimpHistograms(fHist.fSimp[i],Form("Hist/%s",folder_name));
    }
  }
//-----------------------------------------------------------------------------
// book Genp histograms
//-----------------------------------------------------------------------------
  int book_genp_histset[kNGenpHistSets];
  for (int i=0; i<kNGenpHistSets; i++) book_genp_histset[i] = 0;

  book_genp_histset[0] = 1;		// all particles

  for (int i=0; i<kNGenpHistSets; i++) {
    if (book_genp_histset[i] != 0) {
      sprintf(folder_name,"gen_%i",i);
      fol = (TFolder*) hist_folder->FindObject(folder_name);
      if (! fol) fol = hist_folder->AddFolder(folder_name,folder_name);
      fHist.fGenp[i] = new GenpHist_t;
      BookGenpHistograms(fHist.fGenp[i],Form("Hist/%s",folder_name));
    }
  }
//-----------------------------------------------------------------------------
// book CRV pulse histograms
//-----------------------------------------------------------------------------
  int book_pulse_histset[kNCrvPulseHistSets];
  for (int i=0; i<kNCrvPulseHistSets; i++) book_pulse_histset[i] = 0;

  book_pulse_histset[0] = 1;		// all particles

  for (int i=0; i<kNCrvPulseHistSets; i++) {
    if (book_pulse_histset[i] != 0) {
      sprintf(folder_name,"crvp_%i",i);
      fol = (TFolder*) hist_folder->FindObject(folder_name);
      if (! fol) fol = hist_folder->AddFolder(folder_name,folder_name);
      fHist.fCrvPulse[i] = new CrvPulseHist_t;
      BookCrvPulseHistograms(fHist.fCrvPulse[i],Form("Hist/%s",folder_name));
    }
  }
//-----------------------------------------------------------------------------
// book CRV cluster histograms
//-----------------------------------------------------------------------------
  int book_crvc_histset[kNCrvClusterHistSets];
  for (int i=0; i<kNCrvClusterHistSets; i++) book_crvc_histset[i] = 0;

  book_crvc_histset[0] = 1;		// all particles

  for (int i=0; i<kNCrvClusterHistSets; i++) {
    if (book_crvc_histset[i] != 0) {
      sprintf(folder_name,"crvc_%i",i);
      fol = (TFolder*) hist_folder->FindObject(folder_name);
      if (! fol) fol = hist_folder->AddFolder(folder_name,folder_name);
      fHist.fCrvCluster[i] = new CrvClusterHist_t;
      BookCrvClusterHistograms(fHist.fCrvCluster[i],Form("Hist/%s",folder_name));
    }
  }
}


//-----------------------------------------------------------------------------
// need MC truth branch
//-----------------------------------------------------------------------------
void TCrvAnaModule::FillEventHistograms(EventHist_t* Hist) {
  //  double            cos_th(-2.), p(-1.);
  double            xv(-1.e6), yv(-1.e6), rv(-1.e6), zv(-1.e6);
  TLorentzVector    mom;

  if (fParticle) {
    fParticle->Momentum(mom);
    //    p      = mom.P();
    //    cos_th = mom.Pz()/p;
    xv = fParticle->Vx()+3904.;
    yv = fParticle->Vy();
    rv = sqrt(xv*xv+yv*yv);
    zv = fParticle->Vz();
  }

  //  Hist->fEleMom->Fill(p);
  //  Hist->fEleCosTh->Fill(cos_th);
  Hist->fRv->Fill(rv);
  Hist->fZv->Fill(zv);

  Hist->fNCrvClusters->Fill(fNCrvClusters);
  Hist->fNCrvCoincidences->Fill(fNCrvCoincidences);
  Hist->fNCrvPulses->Fill(fNCrvPulses);
}

//-----------------------------------------------------------------------------
void TCrvAnaModule::FillGenpHistograms(GenpHist_t* Hist, TGenParticle* Genp) {
  int    gen_id;
  float  p, cos_th, z0, t0, r0, x0, y0;

  TLorentzVector mom, v;

  Genp->Momentum(mom);
  //  Genp->ProductionVertex(v);

  p      = mom.P();
  cos_th = mom.CosTheta();

  x0     = Genp->Vx()+3904.;
  y0     = Genp->Vy();

  z0     = Genp->Vz();
  t0     = Genp->T();
  r0     = sqrt(x0*x0+y0*y0);
  gen_id = Genp->GetStatusCode();

  Hist->fPdgCode[0]->Fill(Genp->GetPdgCode());
  Hist->fPdgCode[1]->Fill(Genp->GetPdgCode());
  Hist->fGenID->Fill(gen_id);
  Hist->fZ0->Fill(z0);
  Hist->fT0->Fill(t0);
  Hist->fR0->Fill(r0);
  Hist->fP->Fill(p);
  Hist->fCosTh->Fill(cos_th);
}

//-----------------------------------------------------------------------------
void TCrvAnaModule::FillSimpHistograms(SimpHist_t* Hist, TSimParticle* Simp) {

  Hist->fPdgCode->Fill(Simp->fPdgCode);
  Hist->fMomTargetEnd->Fill(Simp->fMomTargetEnd);
  Hist->fMomTrackerFront->Fill(Simp->fMomTrackerFront);
}

//-----------------------------------------------------------------------------
void TCrvAnaModule::FillCrvClusterHistograms(CrvClusterHist_t* Hist, TCrvCoincidenceCluster* CrvCluster) {

  Hist->fSectorType->Fill(CrvCluster->SectorType());
  Hist->fNPulses->Fill(CrvCluster->NPulses());
  Hist->fNPe->Fill(CrvCluster->NPe());
  Hist->fStartTime->Fill(CrvCluster->StartTime());
  Hist->fEndTime->Fill(CrvCluster->EndTime());

  float width  = CrvCluster->EndTime()-CrvCluster->StartTime();
  Hist->fWidth->Fill(width);

  float x = CrvCluster->Position()->X();
  float y = CrvCluster->Position()->Y();
  float z = CrvCluster->Position()->Z();

  Hist->fXVsZ->Fill(z,x);
  Hist->fYVsZ->Fill(z,y);
}

//-----------------------------------------------------------------------------
void TCrvAnaModule::FillCrvPulseHistograms(CrvPulseHist_t* Hist, TCrvRecoPulse* Pulse) {

  Hist->fNPe->Fill(Pulse->NPe());
  Hist->fNPeHeight->Fill(Pulse->NPeHeight());
  Hist->fNDigis->Fill(Pulse->NDigis());
  Hist->fBar->Fill(Pulse->Bar());
  Hist->fSipm->Fill(Pulse->Sipm());
  Hist->fTime->Fill(Pulse->Time());
  Hist->fHeight->Fill(Pulse->Height());
  Hist->fWidth->Fill(Pulse->Width());
  Hist->fChi2->Fill(Pulse->Chi2());
  Hist->fLeTime->Fill(Pulse->LeTime());

  float dt = Pulse->LeTime()-Pulse->Time();
  Hist->fDt->Fill(dt);
}

//-----------------------------------------------------------------------------
// register data blocks and book histograms
//-----------------------------------------------------------------------------
int TCrvAnaModule::BeginJob() {
//-----------------------------------------------------------------------------
// register data blocks, 'HelixBlock' - OR of TPR and CPR
//-----------------------------------------------------------------------------
  RegisterDataBlock("GenpBlock"       ,"TGenpBlock"         ,&fGenpBlock      );
  RegisterDataBlock("SimpBlock"       ,"TSimpBlock"         ,&fSimpBlock      );
  RegisterDataBlock("CrvClusterBlock" ,"TCrvClusterBlock"   ,&fCrvClusterBlock);
  RegisterDataBlock("CrvPulseBlock"   ,"TCrvPulseBlock"     ,&fCrvPulseBlock  );
//-----------------------------------------------------------------------------
// book histograms
//-----------------------------------------------------------------------------
  BookHistograms();

  return 0;
}


//_____________________________________________________________________________
int TCrvAnaModule::BeginRun() {
  int rn = GetHeaderBlock()->RunNumber();
  TStntuple::Init(rn);
  return 0;
}


//_____________________________________________________________________________
void TCrvAnaModule::FillHistograms() {

  //  double       cos_th (-2.);
  //  float        pfront;

//-----------------------------------------------------------------------------
// event histograms
//-----------------------------------------------------------------------------
  FillEventHistograms(fHist.fEvent[0]);
//-----------------------------------------------------------------------------
// fill GENP histograms
// GEN_0: all particles
//-----------------------------------------------------------------------------
  TGenParticle* genp;
  for (int i=0; i<fNGenp; i++) {
    genp = fGenpBlock->Particle(i);
    FillGenpHistograms(fHist.fGenp[0],genp);
  }
//-----------------------------------------------------------------------------
// Simp histograms
//-----------------------------------------------------------------------------
  if (fSimp) {
    FillSimpHistograms(fHist.fSimp[0],fSimp);
  }
//-----------------------------------------------------------------------------
// CRV histograms
//-----------------------------------------------------------------------------
  int ncrvc = fCrvClusterBlock->NClusters();

  for (int i=0; i<ncrvc; i++) {
    TCrvCoincidenceCluster* ccc = fCrvClusterBlock->Cluster(i);
    FillCrvClusterHistograms(fHist.fCrvCluster[0],ccc);
  }

  int np = fCrvPulseBlock->NPulses();

  for (int i=0; i<np; i++) {
    TCrvRecoPulse* p = fCrvPulseBlock->Pulse(i);
    FillCrvPulseHistograms(fHist.fCrvPulse[0],p);
  }
}



//-----------------------------------------------------------------------------
// 2014-04-30: it looks that reading the straw hits takes a lot of time - 
//              turn off by default by commenting it out
//-----------------------------------------------------------------------------
int TCrvAnaModule::Event(int ientry) {

  //  TEmuLogLH::PidData_t  dat;
  //  TStnTrack*            track;
  //  int                   id_word;
  TLorentzVector        mom;

  //  TDiskCalorimeter::GeomData_t disk_geom;

  fGenpBlock->GetEntry(ientry);
  fSimpBlock->GetEntry(ientry);

  fCrvClusterBlock->GetEntry(ientry);
  if (fUseAllPulses) fCrvPulseBlock->GetEntry(ientry);
//-----------------------------------------------------------------------------
// assume electron in the first particle, otherwise the logic will need to 
// be changed
//-----------------------------------------------------------------------------
  fNGenp         = fGenpBlock->NParticles();
  fNCrvClusters  = fCrvClusterBlock->NClusters();

  fNCrvPulses       = fCrvPulseBlock->NPulses();
  fNCrvCoincidences = 0; // no more coincidences ... fCrvPulseBlock->NCoincidences();

  TGenParticle* genp;
  int           pdg_code, process_code;

  fParticle = NULL;
  for (int i=fNGenp-1; i>=0; i--) {
    genp         = fGenpBlock->Particle(i);
    pdg_code     = genp->GetPdgCode();
    process_code = genp->GetStatusCode();
    if ((abs(pdg_code) == fPdgCode) && (process_code == fProcessCode)) {
      fParticle = genp;
      break;
    }
  }
					// may want to revisit the definition of fSimp
  fSimp     = fSimpBlock->Particle(0);

  //  double p = 0;

  if (fParticle) {
    fParticle->Momentum(mom);
					// this is a kludge, to be removed at the next 
					// ntupling 
    //    p         = mom.P();
    //    fEleE     = sqrt(p*p+0.511*0.511);
  }

  FillHistograms();

  Debug();

  return 0;		       
}

//-----------------------------------------------------------------------------
void TCrvAnaModule::Debug() {

  if (GetDebugBit(3) == 1) {
    // if (fNHelPos[1] > 0) {
    //   GetHeaderBlock()->Print(Form("N(CalHelixFinder helices hel > 0) = %2i",fNHelPos[1]));
    // }
  }

  if (GetDebugBit(4) == 1) {
    // if (fNHelNeg[1] > 0) {
    //   GetHeaderBlock()->Print(Form("N(CalHelixFinder helices hel < 0) = %2i",fNHelNeg[1]));
    // }
  }
}

//_____________________________________________________________________________
int TCrvAnaModule::EndJob() {
  return 0;
}

}
