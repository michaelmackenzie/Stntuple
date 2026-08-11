///////////////////////////////////////////////////////////////////////////////
// clang-format off
///////////////////////////////////////////////////////////////////////////////
#include "TROOT.h"
#include "TInterpreter.h"
#include "TMath.h"
#include "TApplication.h"
#include "TVirtualX.h"

#include "TGMenu.h"
#include "TGMsgBox.h"
#include "TGFrame.h"
#include "TGStatusBar.h"
#include "TGaxis.h"
#include "TText.h"
#include "TGDoubleSlider.h"
#include "TGButton.h"
#include "TGTextEntry.h"
#include "TGTextBuffer.h"
#include "TGLabel.h"

#include "Stntuple/gui/TEvdMainFrame.hh"

#include "Stntuple/gui/TCalView.hh"
#include "Stntuple/gui/TCrvView.hh"

#include "Stntuple/gui/TEvdFrame.hh"
#include "Stntuple/gui/TStnVisManager.hh"
#include "Stntuple/gui/TStnWidgetID.hh"
#include "Stntuple/gui/TStnGeoManager.hh"
#include "Stntuple/gui/TEvdPanel.hh"
#include "Stntuple/gui/TEvdPlane.hh"
#include "Stntuple/gui/TEvdTracker.hh"
#include "Stntuple/gui/TEvdStation.hh"

#include "Stntuple/print/Stntuple_print_functions.hh"

#include <boost/lexical_cast/lexical_cast_old.hpp>

#include "TRACE/tracemf.h"
#define  TRACE_NAME         "TStnVisManager"

// ClassImp(TStnVisManager)

//-----------------------------------------------------------------------------
TStnVisManager::TStnVisManager(const char* Name, const char* Title): TVisManager(Name, Title) {
  if (gROOT->IsBatch()) return;

  //  InitGui(Title);
//-----------------------------------------------------------------------------
// views
//-----------------------------------------------------------------------------
  InitViews();

  fListOfDetectors = new TObjArray(10);

  fMinStation          =  0;
  fMaxStation          = 50;
					// by default, no timing constraints
  fTMin                = -1.e6;
  fTMax                =  1.e6;
  fMinEDep             = -1.e6;
  fMaxEDep             =  1.e6;
  fEvent               = nullptr;

  fSelectedTimeCluster = nullptr;
  fSelectedPhiCluster  = nullptr;

  fDisplayHelices      = 0;
  fDisplayCosmicSeeds  = 0;
  fDisplayTracks       = 1;
  fDisplayOnlyTCHits   = 0;
  fDisplaySimParticles = 0;                    // dont' want all of them by default
  fDisplayStrawHitsXY  = 0;                    // display combo hits 
  fDisplayStrawDigiMC  = 0;                    // not sure what this is, check
  fIgnoreComptonHits   = 0;
  fIgnoreProtonHits    = 0;
  fIgnoreProtons       = 0;

  fMinSimpMomentum     = 60;
  fMaxSimpMomentum     = 1.e10;                // in MeV/c
}

//_____________________________________________________________________________
TStnVisManager::~TStnVisManager() {

  if (!gROOT->IsBatch()) {
//-----------------------------------------------------------------------------
// views are deleted by TVisManager
// cleanup gui
//-----------------------------------------------------------------------------
    // delete fMenuBarHelpLayout;
    // delete fMenuBarItemLayout;
    // delete fMenu;
    // delete fMenuBarLayout;
    // delete fMenuBar;
    // delete fMain;

    delete fListOfDetectors;
  }
}

//_____________________________________________________________________________
TStnVisManager* TStnVisManager::Instance() {
  if (TVisManager::fgInstance != NULL) {
    return (TStnVisManager*) TVisManager::fgInstance;
  }
  else {
    return new TStnVisManager();
  }
}

//-----------------------------------------------------------------------------
// this function also opens windows, so better to have it virtual
//-----------------------------------------------------------------------------
int TStnVisManager::InitGui(const char* Title) {
  fMain = new  TEvdMainFrame(gClient->GetRoot(),200,100,kMainFrame | kVerticalFrame);
//-----------------------------------------------------------------------------
//  create menu bar
//-----------------------------------------------------------------------------
  fMenuBarLayout     = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 1, 1);
  fMenuBarItemLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);
  fMenuBarHelpLayout = new TGLayoutHints(kLHintsTop | kLHintsRight);

  fMenu = new TGPopupMenu(gClient->GetRoot());
  fMenu->AddEntry("&Exit", M_EXIT);
  fMenu->Associate(fMain);

  fMenuHelp = new TGPopupMenu(gClient->GetRoot());
  fMenuHelp->AddEntry("&Contents", M_HELP_CONTENTS);
  fMenuHelp->AddEntry("&Search...", M_HELP_SEARCH);
  fMenuHelp->AddSeparator();
  fMenuHelp->AddEntry("&About", M_HELP_ABOUT);
  fMenuHelp->Associate(fMain);

  fMenuBar = new TGMenuBar(fMain, 1, 1, kHorizontalFrame);
  fMenuBar->AddPopup("&Menu", fMenu, fMenuBarItemLayout);
  fMenuBar->AddPopup("&Help", fMenuHelp, fMenuBarHelpLayout);

  fMain->AddFrame(fMenuBar, fMenuBarLayout);
//-----------------------------------------------------------------------------
// view buttons
//-----------------------------------------------------------------------------
  trkrBtnXY = new TGTextButton(fMain, "Tracker XY", TStnVisManager::kXY);
  trkrBtnXY->Connect("Clicked()", "TStnVisManager", this, "HandleButtons()");
  trkrBtnXY->SetTextJustify(36);
  trkrBtnXY->SetMargins(0, 0, 0, 0);
  trkrBtnXY->SetWrapLength(-1);
  trkrBtnXY->MoveResize(16, 26, 98, 24);

  trkrBtnRZ = new TGTextButton(fMain, "Tracker RZ", TStnVisManager::kRZ);
  trkrBtnRZ->Connect("Clicked()", "TStnVisManager", this, "HandleButtons()");
  trkrBtnRZ->SetTextJustify(36);
  trkrBtnRZ->SetMargins(0, 0, 0, 0);
  trkrBtnRZ->SetWrapLength(-1);
  trkrBtnRZ->MoveResize(16, 58, 98, 24);

  calBtn = new TGTextButton(fMain, "Calorimeter", TStnVisManager::kCal);
  calBtn->Connect("Clicked()", "TStnVisManager", this, "HandleButtons()");
  calBtn->SetTextJustify(36);
  calBtn->SetMargins(0, 0, 0, 0);
  calBtn->SetWrapLength(-1);
  calBtn->MoveResize(16, 90, 98, 24);

  crvBtn = new TGTextButton(fMain, "CRV", TStnVisManager::kCrv);
  crvBtn->Connect("Clicked()", "TStnVisManager", this, "HandleButtons()");
  crvBtn->SetTextJustify(36);
  crvBtn->SetMargins(0, 0, 0, 0);
  crvBtn->SetWrapLength(-1);
  crvBtn->MoveResize(16, 122, 98, 24);

  timeWindowSlider = new TGDoubleHSlider(fMain, 100, kDoubleScaleBoth, TIMESLIDER_ID);
  timeWindowSlider->SetRange(0, 1695);
  timeWindowSlider->SetPosition(400, 1695);
  timeWindowSlider->MoveResize(150, 45, 200, 20);
  timeWindowSlider->Connect("PositionChanged()", "TStnVisManager", this, "HandleSlider()");
	
  timeWindowLowDisp = new TGTextEntry(fMain, timeWindowLowBuff = new TGTextBuffer(10), TIMELOW_DISP);
  timeWindowLowBuff->AddText(0, "400");
  timeWindowLowDisp->MoveResize(150, 70, 40, 20);
  timeWindowLowDisp->Connect("ReturnPressed()", "TStnVisManager", this, "HandleText()");

  timeWindowHighDisp = new TGTextEntry(fMain, timeWindowHighBuff = new TGTextBuffer(10), TIMEHIGH_DISP);
  timeWindowHighBuff->AddText(0, "1695");
  timeWindowHighDisp->MoveResize(310, 70, 40, 20);
  timeWindowHighDisp->Connect("ReturnPressed()", "TStnVisManager", this, "HandleText()");

  TGLabel *sliderLabelLow = new TGLabel(fMain, "0");
  sliderLabelLow->SetTextJustify(36);
  sliderLabelLow->SetMargins(0, 0, 0, 0);
  sliderLabelLow->SetWrapLength(-1);
  sliderLabelLow->MoveResize(140, 25, 30, 20);

  TGLabel *sliderLabelHigh = new TGLabel(fMain, "1695");
  sliderLabelHigh->SetTextJustify(36);
  sliderLabelHigh->SetMargins(0, 0, 0, 0);
  sliderLabelHigh->SetWrapLength(-1);
  sliderLabelHigh->MoveResize(330, 25, 30, 20);

  updaterBtn = new TGTextButton(fMain, "Update", UPDATER_BTN);
  updaterBtn->Connect("Clicked()", "TStnVisManager", this, "HandleButtons()");
  updaterBtn->SetTextJustify(36);
  updaterBtn->SetMargins(0, 0, 0, 0);
  updaterBtn->SetWrapLength(-1);
  updaterBtn->MoveResize(220, 120, 60, 20);
//-----------------------------------------------------------------------------
// final actions
//-----------------------------------------------------------------------------
  fMain->MapSubwindows();
  fMain->Resize(fMain->GetDefaultSize());
  fMain->Resize(400, 150);
  fMain->SetWindowName(Title);
  fMain->MapWindow();

  return 0;
}

//-----------------------------------------------------------------------------
int TStnVisManager::EndRun() {
  if (fEvent == nullptr) return 1;
  else                   return 0;
}

//-----------------------------------------------------------------------------
int TStnVisManager::InitViews() {
  return 0;
}

//_____________________________________________________________________________
TCanvas* TStnVisManager::NewCanvas(const char* Name, const char* Title, int SizeX, int SizeY) {
  TEvdFrame* win = new TEvdFrame(Name, Title, this, 0, SizeX, SizeY);
  TCanvas*c = win->GetCanvas();
  DeclareCanvas(c);
  return c;
}

//-----------------------------------------------------------------------------
int TStnVisManager::GetViewID(const char* View) {

  TString view_id = View;
  view_id.ToLower();

  if      (view_id == "xy"  ) return TStnVisManager::kXY;
  else if (view_id == "rz"  ) return TStnVisManager::kRZ;
  else if (view_id == "tz"  ) return TStnVisManager::kTZ;
  else if (view_id == "phiz") return TStnVisManager::kPhiZ;
  else if (view_id == "cal" ) return TStnVisManager::kCal;
  else if (view_id == "crv" ) return TStnVisManager::kCrv;
  else if (view_id == "vst" ) return TStnVisManager::kVST;
  else if (view_id == "vrz" ) return TStnVisManager::kVRZ;
  else {
    printf("TStnVisManager::%s: ERROR: unknown view type : %s\n",__func__,View);
    return -1;
  }
}

//-----------------------------------------------------------------------------
void TStnVisManager::OpenView(const char* View) {

  TString view_id = View;
  view_id.ToLower();

  if      (view_id == "xy"  ) OpenTrkXYView();
  else if (view_id == "rz"  ) OpenTrkRZView();
  else if (view_id == "tz"  ) OpenTrkTZView();
  else if (view_id == "phiz") OpenPhiZView ();
  else if (view_id == "cal" ) OpenCalView  ();
  else if (view_id == "crv" ) OpenCrvView  ();
  else if (view_id == "vst" ) OpenVSTView  ();
  else if (view_id == "vrz" ) OpenVRZView  ();
  else {
    printf("TStnVisManager::OpenView: ERROR: unknown view type : %s\n",View);
  }
}

//-----------------------------------------------------------------------------
void TStnVisManager::OpenView(TStnView* Mother, int Px1, int Py1, int Px2, int Py2) {
  int vtype = Mother->Type();

  if      (vtype == TStnVisManager::kXY  ) OpenTrkXYView(Mother,Px1,Py1,Px2,Py2);
  else if (vtype == TStnVisManager::kRZ  ) OpenTrkRZView(Mother,Px1,Py1,Px2,Py2);
  else if (vtype == TStnVisManager::kTZ  ) OpenTrkTZView(Mother,Px1,Py1,Px2,Py2);
  else if (vtype == TStnVisManager::kPhiZ) OpenPhiZView (Mother,Px1,Py1,Px2,Py2);
  else if (vtype == TStnVisManager::kCal ) OpenCalView  (Mother,Px1,Py1,Px2,Py2);
  else if (vtype == TStnVisManager::kCrv ) OpenCrvView  (Mother,Px1,Py1,Px2,Py2);
  else if (vtype == TStnVisManager::kVST ) OpenVSTView  (Mother,Px1,Py1,Px2,Py2);
  else if (vtype == TStnVisManager::kVRZ ) OpenVRZView  (Mother,Px1,Py1,Px2,Py2);
  else {
    printf("TStnVisManager::OpenView: ERROR: unknown view type : %i\n",vtype);
  }
}

//-----------------------------------------------------------------------------
Int_t TStnVisManager::OpenTrkXYView() {
  // open new XY view of the detector with the default options

  int n = fListOfCanvases->GetSize();

  char name[100], title[100];

  sprintf(name, "xy_view_%i", n);
  sprintf(title, "XY view number %i", n);

  TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kXY, 800+TEvdFrame::fGroupFrameWidth, 800);
  TCanvas* c = win->GetCanvas();
  fListOfCanvases->Add(c);

  TString name1(name);
  name1 += "_1";
  TPad* p1 = (TPad*) c->FindObject(name1);
  p1->Range(-1000., -1000., 1000., 1000.);
  p1->cd();
					// should be the only one
  TStnView* v = FindView(TStnVisManager::kXY,-1);
  v->Draw();

  TString name_title(name);
  name1 += "_title";
  TPad* title_pad = (TPad*) c->FindObject(name_title);
  title_pad->cd();
  fTitleNode->Draw();

  c->Modified();
  c->Update();
  return 0;
}

//_____________________________________________________________________________
Int_t TStnVisManager::OpenTrkXYView(TStnView* Mother, Axis_t x1, Axis_t y1, Axis_t x2, Axis_t y2) {
	// open new XY view of the detector with the default options

  int n = fListOfCanvases->GetSize();

  char name[100], title[100];

  sprintf(name, "xy_view_%i", n);
  sprintf(title, "XY view number %i", n);

  // try to preserve the aspect ratio
  Int_t   xsize, ysize;

  xsize = x2-x1;
  ysize = (int) (xsize*abs((y2 - y1)/(x2 - x1)) + 20);

  // TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kXY, xsize+TEvdFrame::fGroupFrameWidth, ysize);

  xsize = (800./ysize)*xsize;
  TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kTZ, xsize+TEvdFrame::fGroupFrameWidth, 800);

  TCanvas* c = win->GetCanvas();
  fListOfCanvases->Add(c);

  TString name1(name);
  name1 += "_1";
  TPad* p1 = (TPad*) c->FindObject(name1);
  p1->Range(x1, y1, x2, y2);
  p1->cd();
  Mother->Draw();

  TString name_title(name);
  name1 += "_title";
  TPad* title_pad = (TPad*) c->FindObject(name_title);
  title_pad->cd();
  fTitleNode->Draw();

  c->Modified();
  c->Update();
  return 0;
}

//-----------------------------------------------------------------------------
// open new RZ view of the detector with the default options
//-----------------------------------------------------------------------------
Int_t TStnVisManager::OpenTrkRZView() {

  int n = fListOfCanvases->GetSize();

  char name[100], title[100];

  sprintf(name, "rz_view_%i", n);
  sprintf(title, "RZ view number %i", n);

  TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kRZ, 1300+TEvdFrame::fGroupFrameWidth, 500);

  TCanvas* c = win->GetCanvas();
  fListOfCanvases->Add(c);

  TString name1(name);
  name1 += "_1";
  TPad* p1 = (TPad*) c->FindObject(name1);
  //  p1->Range(8500.,-200.,12500.,800.);
  p1->Range(-2000., -300., 3000., 700.);
  p1->cd();
//-----------------------------------------------------------------------------
// find and draw the view itself
//-----------------------------------------------------------------------------
  TStnView* v = FindView(TStnVisManager::kRZ,-1);
  if (v) v->Draw();
//-----------------------------------------------------------------------------
// draw title
//-----------------------------------------------------------------------------
  TString name_title(name);
  name1 += "_title";
  TPad* title_pad = (TPad*) c->FindObject(name_title);
  title_pad->cd();
  fTitleNode->Draw();

  c->Modified();
  c->Update();
  return 0;
}

//-----------------------------------------------------------------------------
// open new RZ view of the detector with the default options
//-----------------------------------------------------------------------------
Int_t TStnVisManager::OpenTrkRZView(TStnView* Mother, Axis_t x1, Axis_t y1, Axis_t x2, Axis_t y2) {

  int n = fListOfCanvases->GetSize();

  char name[100], title[100];

  sprintf(name, "rz_view_%i", n);
  sprintf(title, "RZ view number %i", n);
//-----------------------------------------------------------------------------
// try to preserve the aspect ratio
//-----------------------------------------------------------------------------
  Int_t   xsize, ysize;

  xsize = x2-x1;
  ysize = (Int_t) (xsize*TMath::Abs((y2 - y1) / (x2 - x1)) + 20);

  TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kRZ, xsize+TEvdFrame::fGroupFrameWidth, ysize);
  TCanvas* c = win->GetCanvas();
  fListOfCanvases->Add(c);

  TString name1(name);
  name1 += "_1";
  TPad* p1 = (TPad*) c->FindObject(name1);
  p1->Range(x1, y1, x2, y2);
  p1->cd();
  Mother->Draw();

  TString name_title(name);
  name1 += "_title";
  TPad* title_pad = (TPad*) c->FindObject(name_title);
  title_pad->cd();
  fTitleNode->Draw();

  c->Modified();
  c->Update();
  return 0;
}

//-----------------------------------------------------------------------------
// open new TrkTZ view of the detector with the default options
//-----------------------------------------------------------------------------
int TStnVisManager::OpenTrkTZView() {
  // open new TZ view of the detector with the default options

  int n = fListOfCanvases->GetSize();

  char name[100], title[100];

  sprintf(name,  "zt_view_%i", n);
  sprintf(title, "ZT view number %i", n);

  TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kXY, 1100+TEvdFrame::fGroupFrameWidth, 800);
  TCanvas* c = win->GetCanvas();
  fListOfCanvases->Add(c);

  TString name1(name);
  name1 += "_1";
  TPad* p1 = (TPad*) c->FindObject(name1);
  p1->Range(-1700.,0., 1700., fEWLength);
  p1->cd();

  TStnView* v = FindView(TStnVisManager::kTZ,-1);

  v->Draw();

  TString name_title(name);
  name1 += "_title";
  TPad* title_pad = (TPad*) c->FindObject(name_title);
  title_pad->cd();
  if (fTitleNode) fTitleNode->Draw();

  c->Modified();
  c->Update();
  return 0;
}

//-----------------------------------------------------------------------------
int TStnVisManager::OpenTrkTZView(TStnView* Mother, Axis_t x1, Axis_t y1, Axis_t x2, Axis_t y2) {
	// open new XY view of the detector with the default options

  int n = fListOfCanvases->GetSize();

  char name[100], title[100];

  sprintf(name,  "zt_view_%i", n);
  sprintf(title, "ZT view number %i", n);

  // try to preserve the aspect ratio
  Int_t   xsize, ysize;

  xsize = Mother->GetPx2()-Mother->GetPx1();
  ysize = Mother->GetPy2()-Mother->GetPy1() + 20;

  // TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kTZ, xsize+TEvdFrame::fGroupFrameWidth, ysize);
  xsize = (800./ysize)*xsize;
  TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kTZ, xsize+TEvdFrame::fGroupFrameWidth, 800);

  TCanvas* c = win->GetCanvas();
  fListOfCanvases->Add(c);

  TString name1(name);
  name1 += "_1";
  TPad* p1 = (TPad*) c->FindObject(name1);
  p1->Range(x1, y1, x2, y2);
  p1->cd();
  Mother->Draw();

  TString name_title(name);
  name1 += "_title";
  TPad* title_pad = (TPad*) c->FindObject(name_title);
  title_pad->cd();
  if (fTitleNode) fTitleNode->Draw();

  c->Modified();
  c->Update();
  return 0;
}

//-----------------------------------------------------------------------------
// open new PhiZ view of the detector with the default options
//-----------------------------------------------------------------------------
int TStnVisManager::OpenPhiZView() {
  // open new TZ view of the detector with the default options

  int n = fListOfCanvases->GetSize();

  char name[100], title[100];

  sprintf(name,  "phiz_view_%i", n);
  sprintf(title, "PhiZ view number %i", n);

  TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kXY, 1100+TEvdFrame::fGroupFrameWidth, 800);
  TCanvas* c = win->GetCanvas();
  fListOfCanvases->Add(c);

  TString name1(name);
  name1 += "_1";
  TPad* p1 = (TPad*) c->FindObject(name1);
  p1->Range(-1600., -M_PI, 1600., M_PI);
  p1->cd();

  TStnView* v = FindView(TStnVisManager::kPhiZ,-1);

  v->Draw();

  TString name_title(name);
  name1 += "_title";
  TPad* title_pad = (TPad*) c->FindObject(name_title);
  title_pad->cd();
  if (fTitleNode) fTitleNode->Draw();

  c->Modified();
  c->Update();
  return 0;
}

//-----------------------------------------------------------------------------
// open new PhiZ view of the detector 
//-----------------------------------------------------------------------------
int TStnVisManager::OpenPhiZView(TStnView* Mother, Axis_t x1, Axis_t y1, Axis_t x2, Axis_t y2) {

  int n = fListOfCanvases->GetSize();

  char name[100], title[100];

  sprintf(name,  "phiz_view_%i", n);
  sprintf(title, "PHIZ view number %i", n);

  // try to preserve the aspect ratio
  Int_t   xsize, ysize;

  xsize = x2-x1;
  ysize = (int) (xsize*abs((y2 - y1)/(x2 - x1)) + 20);

  // TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kTZ, xsize+TEvdFrame::fGroupFrameWidth, ysize);
  xsize = (800./ysize)*xsize;
  TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kPhiZ, xsize+TEvdFrame::fGroupFrameWidth, 800);

  TCanvas* c = win->GetCanvas();
  fListOfCanvases->Add(c);

  TString name1(name);
  name1 += "_1";
  TPad* p1 = (TPad*) c->FindObject(name1);
  p1->Range(x1, y1, x2, y2);
  p1->cd();
  Mother->Draw();

  TString name_title(name);
  name1 += "_title";
  TPad* title_pad = (TPad*) c->FindObject(name_title);
  title_pad->cd();
  if (fTitleNode) fTitleNode->Draw();

  c->Modified();
  c->Update();
  return 0;
}

//_____________________________________________________________________________
Int_t TStnVisManager::OpenCalView() {
  // open new calorimeter view of the detector with the default options
  // start from the disk calorimeter

  int n = fListOfCanvases->GetSize();

  char name[100], title[100];

  sprintf(name, "cal_view_%i", n);
  sprintf(title, "CAL view number %i", n);

  TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kCal, 1150+TEvdFrame::fGroupFrameWidth, 600);
  TCanvas*   c = win->GetCanvas();
  fListOfCanvases->Add(c);

  TString name1(name);
  name1 += "_1";
  TPad* p1 = (TPad*) c->FindObject(name1);
//-----------------------------------------------------------------------------
// the disk calorimeter view has two pads, one per disk
// the vane-based calorimeter display should have 4 pads in this view
// divide horisontally
//-----------------------------------------------------------------------------
  p1->Divide(2, 1);
					// ranges in mm
  for (int i=0; i<2; i++) {
    p1->cd(i+1);
    gPad->Range(-800., -800., 800., 800.);
    TCalView* v = (TCalView*) FindView(TStnVisManager::kCal,i);
    if (v) {
      //      v->SetPad(gPad);
      v->Draw();
      gPad->Modified();
    }
  }
					// draw title
  TString name_title(name);
  name1 += "_title";
  TPad* title_pad = (TPad*) c->FindObject(name_title);
  title_pad->cd();
  fTitleNode->Draw();

  c->Modified();
  c->Update();
  return 0;
}

//_____________________________________________________________________________
Int_t TStnVisManager::OpenCalView(TStnView* Mother, Axis_t x1, Axis_t y1, Axis_t x2, Axis_t y2) {
  // open new calorimeter view of the detector with the default options

  //   int n = fListOfCanvases->GetSize();

  //   char name[100], title[100];

  //   sprintf(name,"ces_view_%i",n);
  //   sprintf(title,"CES view number %i",n);

  // 				// try to preserve the aspect ratio
  //   Int_t   xsize, ysize;

  //   xsize = 540;
  //   ysize = (Int_t) (xsize*TMath::Abs((y2-y1)/(x2-x1))+20);

  //   TEvdFrame* win = new TEvdFrame(name, title, kCesStripView, xsize,ysize);
  //   TCanvas* c = win->GetCanvas();
  //   fListOfCanvases->Add(c);
  //   c->Divide(1,2);

  //   TString name1(name);
  //   name1 += "_1";
  //   TPad* p1 = (TPad*) c->FindObject(name1);

  //   p1->Divide(2,1);

  //   p1->cd(1);
  //   gPad->Range(x1,y1,x2,y2);

  //   fCalSectionView[0]->Draw();

  //   p1->cd(2);
  //   gPad->Range(x1,y1,x2,y2);
  //   fCalSectionView[1]->Draw();

  //   TString name_title(name);
  //   name1 += "_title";
  //   TPad* title_pad = (TPad*) c->FindObject(name_title);
  //   title_pad->cd();
  //   fTitleNode->Draw();

  //   c->Modified();
  //   c->Update();
  return 0;
}

//_____________________________________________________________________________
Int_t TStnVisManager::OpenCrvView() {
  TText Tl;
  Tl.SetTextSize(0.2);

  int n = fListOfCanvases->GetSize();

  char name[100], title[100];

  sprintf(name, "crv_view_%i", n);
  sprintf(title, "CRV view number %i", n);

  TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kCrv, 1700+TEvdFrame::fGroupFrameWidth, 600);
  TCanvas*   c = win->GetCanvas();
  c->SetFixedAspectRatio(kTRUE);
  fListOfCanvases->Add(c);

  TString name1(name);
  name1 += "_1";
  TPad* p1 = (TPad*) c->FindObject(name1);

  p1->Divide(1, 10, 0.003, 0.003);
  // ranges in mm
  p1->cd(1);
  gPad->Range(2698., -6570., 18750., -6415.); // Right CRV sans TS region
  gPad->SetFixedAspectRatio(kTRUE);

  TCrvView* crv;

  crv = (TCrvView*) FindView(TStnVisManager::kCrv,0);
  crv->SetPad(gPad);
  crv->Draw();
  Tl.DrawText(3000, -6565, "RIGHT");
  gPad->SetEditable(kFALSE);
  gPad->Modified();
		
  p1->cd(2);
  gPad->Range(2698., -1420., 18750., -1260.); // Left CRV
  gPad->SetFixedAspectRatio(kTRUE);
  crv = (TCrvView*) FindView(TStnVisManager::kCrv,1);
  crv->SetPad(gPad);
  crv->Draw();
  Tl.DrawText(3000, -1420, "LEFT");
  gPad->SetEditable(kFALSE);
  gPad->Modified();

  p1->cd(3);
  gPad->Range(2698., 2560., 18750., 2715.); // Top DS CRV
  gPad->SetFixedAspectRatio(kTRUE);
  crv = (TCrvView*) FindView(TStnVisManager::kCrv,2);
  crv->SetPad(gPad);
  crv->Draw();
  Tl.DrawText(3000, 2560, "TOP DS");
  gPad->SetEditable(kFALSE);
  gPad->Modified();

  //Axis Pad
  p1->cd(4);
  gPad->Range(2698., 0., 18750., 10.);
  TGaxis *a1 = new TGaxis(2698., 7., 18750., 7., 2698., 18750., 50510, "");
  a1->SetName("Zaxis");
  a1->SetTitle("Z (mm)");
  a1->SetLabelSize(0.3);
  a1->SetTitleSize(0.3);
  a1->SetLabelOffset(0.1);
  a1->SetTitleOffset(0.3);
  a1->SetTickSize(0.1);
  a1->Draw();
  gPad->SetEditable(kFALSE);
  gPad->Modified();

  p1->cd(5);
  gPad->Range(-2220., -6570., 2750., -6415.); // Right CRV TS region
  gPad->SetFixedAspectRatio(kTRUE);
  //fCrvView[0]->SetPad(gPad);
  crv = (TCrvView*) FindView(TStnVisManager::kCrv,0);
  crv->Draw("crv");
  Tl.DrawText(-2150, -6565, "RIGHT");
  gPad->SetEditable(kFALSE);
  gPad->Modified();

  p1->cd(6);
  gPad->Range(-2220., 2560., 2750., 2715.); // Top TS CRV
  gPad->SetFixedAspectRatio(kTRUE);
  crv = (TCrvView*) FindView(TStnVisManager::kCrv,5);
  crv->SetPad(gPad);
  crv->Draw();
  Tl.DrawText(-2150, 2560, "TOP TS");
  gPad->SetEditable(kFALSE);
  gPad->Modified();

  //Axis Pad
  p1->cd(7);
  gPad->Range(-2220., 0., 2750., 10.);
  TGaxis *a2 = new TGaxis(-2220., 7., 2750., 7., -2220., 2750., 50510, "");
  a2->SetName("Zaxis2");
  a2->SetTitle("Z (mm)");
  a2->SetLabelSize(0.3);
  a2->SetTitleSize(0.3);
  a2->SetLabelOffset(0.1);
  a2->SetTitleOffset(0.3);
  a2->SetTickSize(0.1);
  a2->Draw();
  gPad->SetEditable(kFALSE);
  gPad->Modified();	
		

  p1->cd(8);
  gPad->Range(-2220., 18695., 2780., 18970.); // Downstream CRV
  gPad->SetFixedAspectRatio(kTRUE);
  crv = (TCrvView*) FindView(TStnVisManager::kCrv,3);
  crv->SetPad(gPad);
  crv->Draw();
  gPad->SetEditable(kFALSE);
  gPad->Modified();

  p1->cd(9);
  gPad->Range(-2220., -2415., 2780., -2260.); // Upstream CRV
  gPad->SetFixedAspectRatio(kTRUE);
  crv = (TCrvView*) FindView(TStnVisManager::kCrv,4);
  crv->SetPad(gPad);
  crv->Draw();
  Tl.DrawText(-2000., -2300., "DWNSTRM");
  Tl.DrawText(   50., -2340., "UPSTRM");
  gPad->SetEditable(kFALSE);
  gPad->Modified();


  p1->cd(10);
  gPad->Range(-2220., 0., 2800., 10.);
  TGaxis *a3 = new TGaxis(-2220., 7., 2780., 7., -2220, 2780, 50510, "");
  a3->SetName("Yaxis");
  a3->SetTitle("Y (mm)");
  a3->SetLabelSize(0.3);
  a3->SetTitleSize(0.3);
  a3->SetLabelOffset(0.2);
  a3->SetTitleOffset(0.3);
  a3->SetTickSize(0.1);
  a3->Draw();
  gPad->SetEditable(kFALSE);
  gPad->Modified();

  // draw title
  TString name_title(name);
  name1 += "_title";
  TPad* title_pad = (TPad*) c->FindObject(name_title);
  title_pad->cd();
  fTitleNode->Draw();
  title_pad->SetEditable(kFALSE);

  c->cd();
  gPad->SetEditable(kFALSE);
  c->Modified();
  c->Update();
  return 0;
}

//-----------------------------------------------------------------------------
int TStnVisManager::OpenCrvView(TStnView* Mother, Axis_t x1, Axis_t y1, Axis_t x2, Axis_t y2) {

  int n = fListOfCanvases->GetSize();
  
  char name[100], title[100];
  
  sprintf(name , "crv_view_%i", n);
  sprintf(title, "CRV view number %i", n);

  // try to preserve the aspect ration
  Int_t   xsize, ysize;

  xsize = 700;
  ysize = (Int_t) (xsize*TMath::Abs((y2 - y1) / (x2 - x1)) + 20);

  TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kCrv, xsize+TEvdFrame::fGroupFrameWidth, ysize);
  TCanvas* c = win->GetCanvas();
  fListOfCanvases->Add(c);

  TString name1(name);
  name1 += "_1";
  TPad* p1 = (TPad*) c->FindObject(name1);
  p1->Range(x1, y1, x2, y2);
  p1->cd();
  Mother->Draw();

  TString name_title(name);
  name1 += "_title";
  TPad* title_pad = (TPad*) c->FindObject(name_title);
  title_pad->cd();
  fTitleNode->Draw();

  c->Modified();
  c->Update();
  return 0;
}

//-----------------------------------------------------------------------------
// open new RZ view of the detector with the default options
//-----------------------------------------------------------------------------
int TStnVisManager::OpenVSTView() {
  // open new TZ view of the detector with the default options

  int n = fListOfCanvases->GetSize();

  char name[100], title[100];

  sprintf(name,  "vst_view_%i", n);
  sprintf(title, "VST view number %i", n);

  TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kVST, 1100+TEvdFrame::fGroupFrameWidth, 760);
  TCanvas* c = win->GetCanvas();
  fListOfCanvases->Add(c);

  TString name1(name);
  name1 += "_1";
  TPad* p1 = (TPad*) c->FindObject(name1);
  p1->Range(-1600., 0., 1600., 1800.);
  p1->cd();

  TStnView* v = FindView(TStnVisManager::kVST,-1);

  v->Draw();

  TString name_title(name);
  name1 += "_title";
  TPad* title_pad = (TPad*) c->FindObject(name_title);
  title_pad->cd();
  if (fTitleNode) fTitleNode->Draw();

  c->Modified();
  c->Update();
  return 0;
}

//-----------------------------------------------------------------------------
int TStnVisManager::OpenVSTView(TStnView* Mother, Axis_t x1, Axis_t y1, Axis_t x2, Axis_t y2) {

  int n = fListOfCanvases->GetSize();
  
  char name[100], title[100];
  
  sprintf(name , "vst_view_%i", n);
  sprintf(title, "VST view number %i", n);

  // try to preserve the aspect ration
  Int_t   xsize, ysize;

  xsize = 700;
  ysize = (Int_t) (xsize*TMath::Abs((y2 - y1) / (x2 - x1)) + 20);

  TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kVST, xsize+TEvdFrame::fGroupFrameWidth, ysize);
  TCanvas* c = win->GetCanvas();
  fListOfCanvases->Add(c);

  TString name1(name);
  name1 += "_1";
  TPad* p1 = (TPad*) c->FindObject(name1);
  p1->Range(x1, y1, x2, y2);
  p1->cd();
  Mother->Draw();

  TString name_title(name);
  name1 += "_title";
  TPad* title_pad = (TPad*) c->FindObject(name_title);
  title_pad->cd();
  fTitleNode->Draw();

  c->Modified();
  c->Update();
  return 0;
}

//-----------------------------------------------------------------------------
// open new RZ view of the detector with the default options
//-----------------------------------------------------------------------------
Int_t TStnVisManager::OpenVRZView() {

  int n = fListOfCanvases->GetSize();

  char name[100], title[100];

  sprintf(name, "vrz_view_%i", n);
  sprintf(title, "VRZ view number %i", n);

  TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kVRZ, 300+TEvdFrame::fGroupFrameWidth,1050);
  TCanvas* c = win->GetCanvas();
  fListOfCanvases->Add(c);

  TString name1(name);
  name1 += "_1";
  TPad* p1 = (TPad*) c->FindObject(name1);
//-----------------------------------------------------------------------------
// display 12 panels together
// VRZ: a plane has 6 panels, each panel has a view and is displayed on a separate pad
// need tracker as we need position of the panel
//-----------------------------------------------------------------------------
  int station(0); // so far, just one for VST
  // TStnGeoManager* gm = TStnGeoManager::Instance();
  // stntuple::TEvdTracker* vt    = gm->GetTracker();
  int nfaces  = 4; // 6
  int np_face = 3; // 2
  p1->Divide(nfaces, np_face);
					// ranges in mm
  for (int face=0; face<nfaces; ++face) {
    for (int ip=0; ip<np_face; ++ip) {
      int ipad = nfaces*ip+face;              // for now, assume one station , otherwise 12*station + ...
      p1->cd(ipad+1);
//-----------------------------------------------------------------------------
// display sequence: first, 3 panels of zface=0 view, then - zface=1 ...etc
// pick up the panel by face and the panel number within the face...
//-----------------------------------------------------------------------------
      int ipln, ipnl;
      stntuple::TEvdTracker::ConvertPanelGeoIndices(station,face,ip,ipln,ipnl);
        
      //      stntuple::TEvdPanel* panel = vt->Station(0)->Plane(ipln)->Panel(ipnl);
//-----------------------------------------------------------------------------
// this assumes that we are displaying in the local reference frame of the panel
// less sure about the rotation..
// in which frame do we want to display ? need to transform tracks 
//-----------------------------------------------------------------------------
//      gPad->Range(panel->Pos()->Z()-40., 350., panel->Pos()->Z()+40., 700.);
      gPad->Range(-40,-200, 40,200);
      int geo_id  = 12*station+6*ipln+ipnl;
      TStnView* v = (TStnView*) FindView(TStnVisManager::kVRZ,geo_id);
      if (v) {
        v->Draw();
        gPad->Modified();
      }
    }
  }
//-----------------------------------------------------------------------------
// draw title
//-----------------------------------------------------------------------------
  TString name_title(name);
  name1 += "_title";
  TPad* title_pad = (TPad*) c->FindObject(name_title);
  title_pad->cd();
  fTitleNode->Draw();

  c->Modified();
  c->Update();
  return 0;
}

//-----------------------------------------------------------------------------
// open new RZ view of the detector with the default options
//-----------------------------------------------------------------------------
Int_t TStnVisManager::OpenVRZView(TStnView* Mother, Axis_t x1, Axis_t y1, Axis_t x2, Axis_t y2) {

  int n = fListOfCanvases->GetSize();

  char name[100], title[100];

  sprintf(name, "vrz_view_%i", n);
  sprintf(title, "VRZ view number %i", n);
//-----------------------------------------------------------------------------
// try to preserve the aspect ratio
//-----------------------------------------------------------------------------
  Int_t   xsize, ysize;

  xsize = x2-x1;
  ysize = (Int_t) (xsize*TMath::Abs((y2 - y1) / (x2 - x1)) + 20);

  xsize = xsize*800./ysize;
  TEvdFrame* win = new TEvdFrame(name, title, this, TStnVisManager::kVRZ, xsize+TEvdFrame::fGroupFrameWidth, 800);
  TCanvas* c = win->GetCanvas();
  fListOfCanvases->Add(c);

  TString name1(name);
  name1 += "_1";
  TPad* p1 = (TPad*) c->FindObject(name1);
  p1->Range(x1, y1, x2, y2);
  p1->cd();
  Mother->Draw();

  TString name_title(name);
  name1 += "_title";
  TPad* title_pad = (TPad*) c->FindObject(name_title);
  title_pad->cd();
  fTitleNode->Draw();

  c->Modified();
  c->Update();
  return 0;
}

//-----------------------------------------------------------------------------
// make sure selections made for teh current event are not applied to 
// the next event
//-----------------------------------------------------------------------------
void TStnVisManager::InitEvent() {
  fSelectedTimeCluster = nullptr;
}

//-----------------------------------------------------------------------------
void TStnVisManager::UpdateViews() {
  TIter it(fListOfCanvases);
  while (TCanvas* c = (TCanvas*) it.Next()) {
    TIter it1(c->GetListOfPrimitives());
    while (TObject* o = it1.Next()) {
      if (o->InheritsFrom("TPad")) {
	TPad* pad = (TPad*) o;
	MarkModified(pad);
      }
    }
    c->Modified();
    c->Update();
  }
}

//-----------------------------------------------------------------------------
void TStnVisManager::CloseWindow() {
	// Called when window is closed via the window manager.

  delete this;
}


//-----------------------------------------------------------------------------
void TStnVisManager::SetStations(int IMin, int IMax) {
  fMinStation = IMin;
  fMaxStation = IMax;
}

//_____________________________________________________________________________
void TStnVisManager::HandleButtons() {
  // Handle different buttons.
  
  TGButton *btn = (TGButton *) gTQSender;
  int id = btn->WidgetId();
  
  switch (id) {
  case TStnVisManager::kXY:
    OpenTrkXYView();
    break;
  case TStnVisManager::kRZ:
    OpenTrkRZView();
    break;
  case TStnVisManager::kCal:
    OpenCalView();
    break;
  case TStnVisManager::kCrv:
    OpenCrvView();
    break;
  case TStnVisManager::kVST:
    OpenVSTView();
    break;
  case UPDATER_BTN:
    for (int i=0; i<6; i++) {
      TCrvView* v = (TCrvView*) FindView(TStnVisManager::kCrv,i);
      v->SetTimeWindow(timeWindowSlider->GetMinPosition(), timeWindowSlider->GetMaxPosition());
    }
    UpdateViews();
    break;
  default:
    printf("Unknown button clicked\n");
    break;
  }
}

//_____________________________________________________________________________
void TStnVisManager::HandleSlider() {
  // Handle slider widget

  Int_t id;
  TGFrame *frm = (TGFrame *) gTQSender;
  TGDoubleSlider *sd = (TGDoubleSlider *) frm;
  id = sd->WidgetId();
  
  switch (id) {
    //case TStnVisManager::TIMESLIDER_ID:
    // Update text boxes with max and min values
    
  case TIMESLIDER_ID:
    timeWindowLowDisp->SetText(boost::lexical_cast<std::string>((int) timeWindowSlider->GetMinPosition()).c_str());
    gClient->NeedRedraw(timeWindowLowDisp);
    
    timeWindowHighDisp->SetText(boost::lexical_cast<std::string>((int) timeWindowSlider->GetMaxPosition()).c_str());
    gClient->NeedRedraw(timeWindowHighDisp);
    break;
  default:
    break;
  }
}

//_____________________________________________________________________________
void TStnVisManager::HandleText() {
  // Handle text entry widgets

  TGTextEntry *te = (TGTextEntry *) gTQSender;
  Int_t id = te->WidgetId();

  float textBoxNum;

  switch (id) {
  case TIMELOW_DISP:
    try{
      textBoxNum = boost::lexical_cast<float>(timeWindowLowDisp->GetText());
      if (textBoxNum < 0 || textBoxNum > timeWindowSlider->GetMaxPosition())
	timeWindowLowDisp->SetText(boost::lexical_cast<std::string>((int) timeWindowSlider->GetMinPosition()).c_str());
      else {
	timeWindowSlider->SetPosition(textBoxNum, timeWindowSlider->GetMaxPosition());
	timeWindowLowDisp->SetText(boost::lexical_cast<std::string>(textBoxNum).c_str());
      }
    }
    catch (boost::bad_lexical_cast &){
      timeWindowLowDisp->SetText(boost::lexical_cast<std::string>((int) timeWindowSlider->GetMinPosition()).c_str());
    }		
    break;
  case TIMEHIGH_DISP:
    try {
      textBoxNum = boost::lexical_cast<float>(timeWindowHighDisp->GetText());
      if (textBoxNum > 1695 || textBoxNum < timeWindowSlider->GetMinPosition())
	timeWindowHighDisp->SetText(boost::lexical_cast<std::string>((int) timeWindowSlider->GetMaxPosition()).c_str());
      else {
	timeWindowSlider->SetPosition(timeWindowSlider->GetMinPosition(), textBoxNum);
	timeWindowHighDisp->SetText(boost::lexical_cast<std::string>(textBoxNum).c_str());
      }
    }
    catch (boost::bad_lexical_cast &){
      timeWindowHighDisp->SetText(boost::lexical_cast<std::string>((int) timeWindowSlider->GetMaxPosition()).c_str());
    }
    break;
  default:
    break;
  }
}

//_____________________________________________________________________________
void TStnVisManager::NextEvent() {
  printf(" TStnVisManager::NextEvent : next event \n");
  gROOT->ProcessLine(".q");
}

//-----------------------------------------------------------------------------
// note missing 's' : 'KalSeeds' vs 'KalSeed' - NodePrint takes the class name...
//-----------------------------------------------------------------------------
void TStnVisManager::PrintColls(const char* Tag) {
  TString tag = Tag;

  try {
    if      (tag == "CosmicTrackSeedColls") print_cts_colls();
    else if (tag == "HelixSeedColls"      ) print_helix_seed_colls();
    else if (tag == "KalSeedColls"        ) print_kalseed_colls();
    else if (tag == "StrawDigiColls"      ) print_sd_colls     ();
    else if (tag == "StrawDigiMCColls"    ) print_sdmc_colls   ();
    else if (tag == "TimeClusterColls"    ) print_tc_colls();
    else if (tag == "ComboHits"           ) FindNode("TrkVisNode")->NodePrint(0,"ComboHit");
    else if (tag == "CosmicTrackSeeds"    ) FindNode("TrkVisNode")->NodePrint(0,"CosmicTrackSeed");
    else if (tag == "Helices"             ) FindNode("HelixVisNode")->NodePrint(0,"HelixSeed");
    else if (tag == "KalSeeds"            ) FindNode("TrkVisNode")->NodePrint(0,"KalSeed");
    else if (tag == "SimParticles"        ) FindNode("TrkVisNode")->NodePrint(0,"SimParticle");
    else if (tag == "StrawDigis"          ) FindNode("TrkVisNode")->NodePrint(0,"StrawDigi"  );
    else if (tag == "StrawHits"           ) FindNode("TrkVisNode")->NodePrint(0,"StrawHit"   );
    else if (tag == "TimeClusters"        ) FindNode("TimeClusterVisNode")->NodePrint(0,"TimeCluster");
    else {
      TLOG(TLVL_WARNING) << "undefined button " << Tag ;
    }
  }
  catch(...) {
    TLOG(TLVL_ERROR) << "failed to print " << Tag;
  }

}

//_____________________________________________________________________________
void TStnVisManager::DoCheckButton() {

  TGButton* btn    = (TGButton *) gTQSender;
  int       id     = btn->WidgetId();
  int       status = (int) btn->IsOn(); 

  printf("TStnVisManager::%s: button ID: %i state: %i\n",__func__,id,status);

  if      (id == kDisplayHelices     ) SetDisplayHelices     (status);
  else if (id == kDisplayTracks      ) SetDisplayTracks      (status);
  else if (id == kDisplaySimParticles) SetDisplaySimParticles(status);
  else if (id == kDisplayOnlyTCHits  ) SetDisplayOnlyTCHits  (status);
  else if (id == kDisplayCosmicSeeds ) SetDisplayCosmicSeeds (status);
  else if (id == kDisplaySH          ) SetDisplayStrawHitsXY (status);
  else if (id == kIgnoreComptonHits  ) SetIgnoreComptonHits  (status);
  else if (id == kIgnoreProtonHits   ) SetIgnoreProtonHits   (status);
  else if (id == kIgnoreProtons      ) SetIgnoreProtons      (status);
  else {
    printf("WARNING: TStnVisManager::DoCheckButton unknown button ID: %i\n",id);
  }

}

//-----------------------------------------------------------------------------
// it is virtual
//-----------------------------------------------------------------------------
void TStnVisManager::DoRadioButton() {

  TGButton* btn = (TGButton*) gTQSender;
  int id = btn->WidgetId();

  EButtonState new_state;

  EButtonState state = btn->GetState();
  if (state == kButtonUp) new_state = kButtonDown;
  else                    new_state = kButtonUp;

  printf("TStnVisManager::%s RADIO button ID: %i state: %i new_state: %i\n",
         __func__,id,state,new_state);
  
  if      (id == M_DISPLAY_SH) { 
    if (new_state == kButtonUp) SetDisplayStrawHitsXY(1);
    else                        SetDisplayStrawHitsXY(0);
  }
  else if (id == M_IGNORE_COMPTON_HITS ) {
    if (new_state == kButtonUp) SetIgnoreComptonHits(1);
    else                        SetIgnoreComptonHits(0);
  }
  else if (id == M_IGNORE_PROTON_HITS ) {
    if (new_state == kButtonUp) SetIgnoreProtonHits(1);
    else                        SetIgnoreProtonHits(0);
  }
  else if (id == M_IGNORE_PROTONS ) {
    if (new_state == kButtonUp) SetIgnoreProtons(1);
    else                        SetIgnoreProtons(0);
  }
  else {
    printf("WARNING: TStnVisManager::DoRadioButton unknown button ID: %i\n",id);
  }

  btn->SetState(new_state);
}

//_____________________________________________________________________________
void TStnVisManager::Quit() {
  printf(" TStnVisManager::Quit  \n");
  gROOT->ProcessLine(".qqqq");
}

