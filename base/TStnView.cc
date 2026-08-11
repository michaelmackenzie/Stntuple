///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
#include "TObjArray.h"
#include "TVirtualX.h"
#include "TText.h"

#include "Stntuple/base/TStnView.hh"
#include "Stntuple/base/TVisNode.hh"
#include "Stntuple/base/TVisManager.hh"

ClassImp(TStnView)

int TStnView::fgDebugLevel(0);

//-----------------------------------------------------------------------------
TStnView::TStnView(int Type, int Index): TNamed("","") {

  fType        = Type;
  fIndex       = Index;
  fMother      = nullptr;
  fCenter      = nullptr;
  fListOfNodes = nullptr;

  fXAxis       = nullptr;
  fYAxis       = nullptr;
}

//-----------------------------------------------------------------------------
TStnView::TStnView(int Type, int Index, const char* Name, const char* Title):
  TNamed(Name,Title)  {
  fType        = Type;
  fIndex       = Index;
  fMother      = nullptr;

  fCenter      = new TMarker(0.,0,kPlus);
  fCenter->SetMarkerColor(kBlue);
  fCenter->SetMarkerSize(3.);

  fXAxis = new TGaxis(0.05,0.05,0.95,0.05,0.,1,512,"+");
  fYAxis = new TGaxis(0.05,0.05,0.05,0.95,0.,1,512,"+");

  fXAxis->SetLabelFont(52);
  fXAxis->SetLabelSize(0.02);

  fYAxis->SetLabelFont(52);
  fYAxis->SetLabelSize(0.02);

  fListOfNodes = new TObjArray();
}

//_____________________________________________________________________________
TStnView::~TStnView() {
  delete fCenter;
  
  fListOfNodes->Delete();
  delete fListOfNodes;

  delete fXAxis;
  delete fYAxis;
}

// //-----------------------------------------------------------------------------
// void TStnView::CloneCombiTrans(const TGeoCombiTrans* T) {
//   fCombiTrans = new TGeoCombiTrans(*T);
// }

//-----------------------------------------------------------------------------
Int_t TStnView::DistancetoPrimitive(Int_t px, Int_t py) {
  //

  int dist, min_dist(9999);

  TVisManager* vm = TVisManager::Instance();

  vm->SetClosestObject    (nullptr,9999);
  vm->SetClosestDetElement(nullptr,9999);
//-----------------------------------------------------------------------------
// find closest object and the corresponding distance
//-----------------------------------------------------------------------------
  Int_t n = GetNNodes();
  for (int i=0; i<n; i++) {
    TVisNode* node =  GetNode(i);
    dist = node->DistancetoPrimitive(px,py);
    if (dist < min_dist) {
      min_dist = dist;
				// closest object may be one of the managed by
				// the node

      vm->SetClosestObject(node->GetClosestObject(),dist);
    }
  }
//-----------------------------------------------------------------------------
// find closest detector element (not sure if I really need this)
//-----------------------------------------------------------------------------
//  if (TDetectorElement::GetMinDist() < vm->GetMinDistDetElement()) {
//     vm->SetClosestDetElement(TDetectorElement::GetClosest(),
// 			     TDetectorElement::GetMinDist());
//  }

  if (vm->GetMinDist() > 10) vm->SetClosestObject(this,0);
//-----------------------------------------------------------------------------
// prepare output
//-----------------------------------------------------------------------------
  gPad->SetSelected(vm->GetClosestObject());
  return vm->GetMinDist();
}

//-----------------------------------------------------------------------------
void TStnView::ExecuteEvent(Int_t event, Int_t px, Int_t py) {
  //

  TVisManager* vm = TVisManager::Instance();

  if (fgDebugLevel > 0) {
    printf(" >>>>>>  TStnView::ExecuteEvent event = %i px:%4i py:%4i\n",event,px,py);
  }
//-----------------------------------------------------------------------------
// if closest object has been identified, execute its action and leave
//-----------------------------------------------------------------------------
  TObject* o = vm->GetClosestObject();
  if (o && (o != this) && (vm->GetMinDist() < 5) ) {
    o->ExecuteEvent(event,px,py);
    return;
  }
//-----------------------------------------------------------------------------
// no closest object, continue hovering over the view
//-----------------------------------------------------------------------------
  Axis_t x, y, x1, x2, y1, y2, dx, dy;
  Axis_t  x1new(0), x2new(0), y1new(0), y2new(0);

  //  double     xx,yy;
  //  int        px, py;
  
  double shift_scale = 0.02;
  double zoom_scale  = 0.05;

  TVirtualX::EBoxMode ebox_mode = TVirtualX::kHollow;
  //  TVirtualX::EBoxMode ebox_mode = TVirtualX::kFilled;

  gVirtualX->SetLineColor(1);
  gVirtualX->SetFillColor(1);
  gVirtualX->SetFillStyle(3003);

  switch (event) {

  case kButton1Down:

    if (fgDebugLevel > 0) printf(" TStnView::ExecuteEvent  kButton1Down\n");

    fPx1 = px;
    fPy1 = py;
    fPx2 = px;
    fPy2 = py;

    gVirtualX->DrawBox(fPx1, fPy1, fPx2, fPy2, ebox_mode);

    break;

  case kButton1Motion:
				// redraw the opaque rectangle
    gPad->SetCursor(kCross);

    gVirtualX->DrawBox(fPx1, fPy1, fPx2, fPy2, ebox_mode);
    fPx2 = px;
    fPy2 = py;
    gVirtualX->DrawBox(fPx1, fPy1, fPx2, fPy2, ebox_mode);

    break;
  case kWheelUp:  /* 5 , zoom in*/
    gPad->GetRange(x1,y1,x2,y2);
    x1new = x1+(x2-x1)/2.*zoom_scale;
    x2new = x2-(x2-x1)/2.*zoom_scale;
    y1new = y1+(y2-y1)/2.*zoom_scale;
    y2new = y2-(y2-y1)/2.*zoom_scale;

    gPad->Range(x1new,y1new,x2new,y2new);
    gPad->Modified();
    gPad->Update();
    break;
  case kWheelDown:  /* 6 */
    gPad->GetRange(x1,y1,x2,y2);
    x1new = x1-(x2-x1)/2.*zoom_scale;
    x2new = x2+(x2-x1)/2.*zoom_scale;
    y1new = y1-(y2-y1)/2.*zoom_scale;
    y2new = y2+(y2-y1)/2.*zoom_scale;

    gPad->Range(x1new,y1new,x2new,y2new);
    gPad->Modified();
    gPad->Update();
    break;
  case kKeyPress:
    if (TStnView::fgDebugLevel > 0) printf(" TStnView::ExecuteEvent kKeyPress: px=%3i py:%i\n",px,py);

    if (px == py) {
      gPad->GetRange(x1,y1,x2,y2);

      if (char(px) == 'z') {            // zoom in
	x1new = x1+(x2-x1)/2.*zoom_scale;
	x2new = x2-(x2-x1)/2.*zoom_scale;
	y1new = y1+(y2-y1)/2.*zoom_scale;
	y2new = y2-(y2-y1)/2.*zoom_scale;
      }
      else if (char(px) == 'Z') { 	// zoom out
	x1new = x1-(x2-x1)/2.*zoom_scale;
	x2new = x2+(x2-x1)/2.*zoom_scale;
	y1new = y1-(y2-y1)/2.*zoom_scale;
	y2new = y2+(y2-y1)/2.*zoom_scale;
      }

      gPad->Range(x1new,y1new,x2new,y2new);
      gPad->Modified();
      gPad->Update();
    }
    break;
  case kButton2Up:
    if (TStnView::fgDebugLevel > 0) printf(" TStnView::ExecuteEvent kButton2Up\n");
    break;
  case kButton2Down:
    if (TStnView::fgDebugLevel > 0) printf("TStnView::ExecuteEvent  kButton2Down\n");
    break;
  case kButton3Up:
    if (TStnView::fgDebugLevel > 0) printf(" TStnView::ExecuteEvent kButton3Up\n");
    break;
  case kButton3Down:
    if (TStnView::fgDebugLevel > 0) printf(" TStnView::ExecuteEvent kButton3Down\n");
    break;
  case kButton1Up:
    printf(" TStnView::ExecuteEvent kButton1Up px,py= %5i %5i\n",px,py);
//-----------------------------------------------------------------------------
// open new window only if something has really been selected (it is a 
// rectangle, but not a occasional click)
//-----------------------------------------------------------------------------
    if ( (fPx1 != fPx2) && (fPy1 != fPy2) ) {
      x1 = gPad->AbsPixeltoX(fPx1);
      y1 = gPad->AbsPixeltoY(fPy1);
      x2 = gPad->AbsPixeltoX(fPx2);
      y2 = gPad->AbsPixeltoY(fPy2);

      if (x1 > x2) {
	x  = x1;
	x1 = x2;
	x2 = x;
      }

      if (y1 > y2) {
	y  = y1;
	y1 = y2;
	y2 = y;
      }
      vm->OpenView(this,(int)x1,(int)y1,(int)x2,(int)y2);
    }
    break;
  case kArrowKeyPress: // 25
//-----------------------------------------------------------------------------
// arrow key pressed 
//-----------------------------------------------------------------------------
    if (TStnView::fgDebugLevel > 0) printf("TStnView: ARROW key pressed  = %3i  py = %3i\n",px,py);
    fCursorX = px;
    fCursorY = py;
//     gPad->GetRange(x1,y1,x2,y2);
//     gPad->Range(x1*1.1,y1*1.1,x2*1.1,y2*1.1);
//    gPad->Modified();
    break;
  case kArrowKeyRelease: // 26
//-----------------------------------------------------------------------------
// arrow key released
//-----------------------------------------------------------------------------
    if (TStnView::fgDebugLevel > 0) printf("TStnView: ARROW key released px = %3i  py = %3i\n",px,py);

    gPad->GetRange(x1,y1,x2,y2);

    if (px == fCursorX) {
					         // up or down arrow
      dy = y2-y1;
      if (fCursorY > py) {
					         // down arrow

	gPad->Range(x1,y1-shift_scale*dy,x2,y2-shift_scale*dy);
      }
      else {
	gPad->Range(x1,y1+shift_scale*dy,x2,y2+shift_scale*dy);
      }
    }
    else if (py == fCursorY) {
					// left or right arrow
      dx = x2-x1;
      if (fCursorX > px) {
					         // left arrow
	gPad->Range(x1+shift_scale*dx,y1,x2+shift_scale*dx,y2);
      }
      else {
	gPad->Range(x1-shift_scale*dx,y1,x2-shift_scale*dx,y2);
      }
    }
    gPad->Modified();
    gPad->Update();

    break;
  default:
    break;
  }

  //   if (event != kMouseLeave) {                  // signal was already emitted for this event
  //     DrawEventStatus(event, px, py);
  //   }
}

//-----------------------------------------------------------------------------
char* TStnView::GetObjectInfo(Int_t Px, Int_t Py) const {
  // need to find the active frame:

  double xx, yy;

  static char info[200];

  xx = gPad->AbsPixeltoX(Px);
  yy = gPad->AbsPixeltoY(Py);
  
  sprintf(info,"Z=%9.3f R=%8.3f",xx,yy);

  return info;
}

//-----------------------------------------------------------------------------
void TStnView::Paint(Option_t* Option) {
  //
  TVisManager* vm = TVisManager::Instance();
  
  if (vm->EndRun() == 1) return; 

  vm->SetCurrentView(this);

  TText txt;
  txt.SetTextFont(52);
  txt.DrawTextNDC(0.75,0.97,GetTitle());

  fCenter->Paint(Option);
//-----------------------------------------------------------------------------
// draw nodes
//-----------------------------------------------------------------------------
  Int_t n = GetNNodes();
  for (int i=0; i<n; i++) {
    TVisNode* node = GetNode(i);
    if (TStnView::fgDebugLevel != 0) {
      printf("TStnView::%s:%5i node:%s\n",__func__, __LINE__,node->GetName());
    }
    node->Paint(Option);
  }
//-----------------------------------------------------------------------------
// pad dimensions in real world coordinates - draw axes
//-----------------------------------------------------------------------------
  double x1 = gPad->GetX1();
  double x2 = gPad->GetX2();
  double y1 = gPad->GetY1();
  double y2 = gPad->GetY2();

  double xmin = x1+(x2-x1)*0.05;
  double xmax = x1+(x2-x1)*0.95;
  double ymin = y1+(y2-y1)*0.05;
  double ymax = y1+(y2-y1)*0.95;

  fXAxis->SetX1(xmin);
  fXAxis->SetX2(xmax);
  fXAxis->SetY1(ymin);
  fXAxis->SetY2(ymin);

  fXAxis->SetWmin(xmin);
  fXAxis->SetWmax(xmax);
  fXAxis->Paint(Option);

  fYAxis->SetX1(xmin);
  fYAxis->SetX2(xmin);
  fYAxis->SetY1(ymin);
  fYAxis->SetY2(ymax);

  fYAxis->SetWmin(ymin);
  fYAxis->SetWmax(ymax);

  fXAxis->Paint("");
  fYAxis->Paint("");

  gPad->Modified();
}

//-----------------------------------------------------------------------------
void    TStnView::SetStations(int I1, int I2) {
  TVisManager* vm = TVisManager::Instance();

  vm->SetStations(I1, I2);
}

//-----------------------------------------------------------------------------
void    TStnView::SetTimeWindow(float TMin, float TMax) {
  TVisManager* vm = TVisManager::Instance();
  vm->SetTimeWindow(TMin,TMax);
}

//-----------------------------------------------------------------------------
void    TStnView::SetMinEDep(float E) {
  TVisManager* vm = TVisManager::Instance();
  vm->SetMinEDep(E);
}

//-----------------------------------------------------------------------------
void    TStnView::SetMaxEDep(float E) {
  TVisManager* vm = TVisManager::Instance();
  vm->SetMaxEDep(E);
}

//-----------------------------------------------------------------------------
void TStnView::SetDebugLevel(int Level) {
  TStnView::fgDebugLevel = Level;
}

//-----------------------------------------------------------------------------
void TStnView::Print(Option_t* Option) const {

  TStnView* v = (TStnView*) this;

  printf("TStnView: %s\n",GetName());

  printf(" fType = %i fIndex = %i\n",fType,fIndex);
  printf(" fPx1 fPy1 fPx2 fPy2 = %5i %5i %5i %5i\n",fPx1,fPy1,fPx2,fPy2);

  int n = v->GetNNodes();

  printf("--------------------- nodes\n");
  for (int i=0; i<n; i++) {
    TVisNode* n = v->GetNode(i);
    printf(" i, node: %3i %-10s\n",i,n->GetName());
    n->Print();
  }
}

