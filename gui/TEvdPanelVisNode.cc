///////////////////////////////////////////////////////////////////////////////
// real base class for Stntuple VisNodes 
///////////////////////////////////////////////////////////////////////////////
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "Offline/RecoDataProducts/inc/ComboHit.hh"
#include "Stntuple/gui/TEvdPanelVisNode.hh"
#include "Stntuple/gui/TStnVisManager.hh"

#include "Stntuple/gui/TEvdStraw.hh"
#include "Stntuple/gui/TEvdPanel.hh"

// #include "trkvst/ana/TEvdPanelData.hh"

// #include "Stntuple/obj/TStrawHitBlock.hh"
// #include "Stntuple/obj/TStrWaveform.hh"

 ClassImp(stntuple::TEvdPanelVisNode)

 namespace stntuple {

//-----------------------------------------------------------------------------
TEvdPanelVisNode::TEvdPanelVisNode(const char* Name, TEvdPanel* Panel): TStnVisNode(Name) {
  fPanel    = Panel;
}

//-----------------------------------------------------------------------------
TEvdPanelVisNode::~TEvdPanelVisNode() {
}

//-----------------------------------------------------------------------------
// so far, assume that there is only one plane
//-----------------------------------------------------------------------------
int TEvdPanelVisNode::InitEvent() {
  //  double sigr(2.5);   // in [mm], for display only
  //  int color(kRed+2), mask(0);
  return 0;
}

//-----------------------------------------------------------------------------
void TEvdPanelVisNode::PaintCrv(Option_t* Option) {
}

//-----------------------------------------------------------------------------
void TEvdPanelVisNode::PaintCal(Option_t* Option) {
}

//-----------------------------------------------------------------------------
void TEvdPanelVisNode::PaintXY (Option_t* Option) {
  fPanel->PaintXY(Option);
}

//-----------------------------------------------------------------------------
void TEvdPanelVisNode::PaintPhiZ(Option_t* Option) {
}

//-----------------------------------------------------------------------------
void TEvdPanelVisNode::PaintRZ (Option_t* Option) {
  fPanel->PaintRZ(Option);
}

//-----------------------------------------------------------------------------
void TEvdPanelVisNode::PaintTZ(Option_t* Option) {
}

//-----------------------------------------------------------------------------
void TEvdPanelVisNode::PaintVST (Option_t* Option) {
  fPanel->PaintVST(Option);
}

//-----------------------------------------------------------------------------
void TEvdPanelVisNode::PaintVRZ (Option_t* Option) {
  fPanel->PaintVRZ(Option);
}

//-----------------------------------------------------------------------------
int  TEvdPanelVisNode::DistancetoPrimitive(Int_t px, Int_t py) {
  // by default, return a large number
  // decide how to deal with 3D views later

  int dist(9999);

  int view  = TStnVisManager::Instance()->GetCurrentView()->Type();
  //  int index = TVstVisManager::Instance()->GetCurrentView()->Index();
 
  if      (view == TStnVisManager::kXY ) dist = DistancetoPrimitiveXY (px,py);
  else if (view == TStnVisManager::kRZ ) dist = DistancetoPrimitiveRZ (px,py);
  else if (view == TStnVisManager::kVST) dist = DistancetoPrimitiveVST(px,py);
  else if (view == TStnVisManager::kVRZ) dist = DistancetoPrimitiveVRZ(px,py);
  else {
    // what is the default?
    //    Warning("Paint",Form("Unknown option %s",option));
  }

  return dist;
}

//-----------------------------------------------------------------------------
int  TEvdPanelVisNode::DistancetoPrimitiveXY(Int_t px, Int_t py) {
  // by default, return a large number
  // decide how to deal with 3D views later

  int  min_dist(9999);
  return min_dist;
}

//-----------------------------------------------------------------------------
int  TEvdPanelVisNode::DistancetoPrimitiveRZ(Int_t px, Int_t py) {
  // by default, return a large number
  // decide how to deal with 3D views later

  int  min_dist(9999);
  return min_dist;
}

//-----------------------------------------------------------------------------
int  TEvdPanelVisNode::DistancetoPrimitiveVST(Int_t px, Int_t py) {
  // by default, return a large number
  // decide how to deal with 3D views later

  int  min_dist(9999);
  return min_dist;
}

//-----------------------------------------------------------------------------
int  TEvdPanelVisNode::DistancetoPrimitiveVRZ(Int_t px, Int_t py) {
  // by default, return a large number
  // decide how to deal with 3D views later

  int      min_dist(9999);
  TObject* closest(nullptr);

  int nstraws = fPanel->NStraws();
  for (int is=0; is<nstraws; is++) {
    TEvdStraw* straw  = fPanel->Straw(is);
    int dist = straw->DistancetoPrimitiveVRZ(px,py);
    if (dist < min_dist) {
      min_dist = dist;
      closest = straw;
    }
  }

  SetClosestObject(closest,min_dist);
  return min_dist;
}

//-----------------------------------------------------------------------------
void TEvdPanelVisNode::Print(const char* Opt) const {
  printf("TEvdPanelVisNode::%s not implemented yet\n",__func__);
}

}
