#pragma once
#include "IControl.h"
#include "src/graph/misc/ParameterCoupling.h"
#include "src/graph/misc/NodeSocket.h"
#include "src/graph/ui/NodeSocketUi.h"

struct NodeUiParam {
  iplug::igraphics::IGraphics* pGraphics;
  const char* pBg;
  float* X;
  float* Y;
  WDL_PtrList<ParameterCoupling>* pParameters;
  WDL_PtrList<NodeSocket>* inSockets;
  WDL_PtrList<NodeSocket>* outSockets;
};

using namespace iplug;
using namespace igraphics;


/**
 * This class represents a Node on the UI, it's seperate to the node itself
 * since it will only exists as long as the UI window is open but is owned by the node
 */
class NodeUi : public IControl {
  WDL_PtrList<ParameterCoupling>* mParameters;
  WDL_PtrList<NodeSocket>* mInSockets;
  WDL_PtrList<NodeSocket>* mOutSockets;
  WDL_PtrList<NodeSocketUi> mInSocketsUi;
  WDL_PtrList<NodeSocketUi> mOutSocketsUi;
public:
  NodeUi(NodeUiParam pParam) :
    IControl(IRECT(0, 0, 0, 0), kNoParameter)
  {
    X = pParam.X;
    Y = pParam.Y;
    mGraphics = pParam.pGraphics;
    mParameters = pParam.pParameters;
    mInSockets = pParam.inSockets;
    mOutSockets = pParam.outSockets;

    mBitmap = mGraphics->LoadBitmap(pParam.pBg, 1, false);
    float w = mBitmap.W();
    float h = mBitmap.H();
    IRECT rect;
    rect.L = *X - w / 2;
    rect.R = *X + w / 2;
    rect.T = *Y - h / 2;
    rect.B = *Y + h / 2;
    SetTargetAndDrawRECTs(rect);
    mBlend = EBlend::Clobber;
  }

  ~NodeUi() {

  }

  void setUp(NodeSocketCallback callback) {
    for (int i = 0; i < mParameters->GetSize(); i++) {
      ParameterCoupling* couple = mParameters->Get(i);
      double value = *(couple->value);
      float px = *X + couple->x - (couple->w * 0.5);
      float py = *Y + couple->y - (couple->h * 0.5);
      IRECT controlPos(px, py, px + couple->w, py + couple->h);
      // use the daw parameter to sync the values if possible
      if (couple->parameterIdx != kNoParameter) {
        couple->control = new IVKnobControl(
          controlPos, couple->parameterIdx
        );
      }
      else {
        // use the callback to get tha value to the dsp, won't allow automation though
        couple->control = new IVKnobControl(
          controlPos, [couple](IControl* pCaller) {
          *(couple->value) = pCaller->GetValue();
        }
        );
      }
      couple->control->SetValue(value);
      mGraphics->AttachControl(couple->control);
      couple->control->SetValue(value);

      // optinally hide the lables etc
      //IVectorBase* vcontrol = dynamic_cast<IVectorBase*>(couple->control);
      //if (vcontrol != nullptr) {
      //  vcontrol->SetShowLabel(false);
      //  vcontrol->SetShowValue(false);
      //}
    }

    for (int i = 0; i < mInSockets->GetSize(); i++) {
      NodeSocketUi* socket = new NodeSocketUi(mGraphics, mInSockets->Get(i), callback);
      mGraphics->AttachControl(socket);
      mInSocketsUi.Add(socket);
    }

    for (int i = 0; i < mOutSockets->GetSize(); i++) {
      NodeSocketUi* socket = new NodeSocketUi(mGraphics, mOutSockets->Get(i), callback);
      mGraphics->AttachControl(socket);
      mOutSocketsUi.Add(socket);
    }
  }

  void cleanUp() {
    for (int i = 0; i < mParameters->GetSize(); i++) {
      ParameterCoupling* param = mParameters->Get(i);
      if (param->control != nullptr) {
        // this also destroys the object
        mGraphics->RemoveControl(param->control, true);
        param->control = nullptr;
      }
    }

    for (int i = 0; i < mInSocketsUi.GetSize(); i++) {
      mGraphics->RemoveControl(mInSocketsUi.Get(i), true);
    }

    for (int i = 0; i < mOutSocketsUi.GetSize(); i++) {
      mGraphics->RemoveControl(mOutSocketsUi.Get(i), true);
    }
  }

  void Draw(IGraphics& g) override {
    // this will just draw the backround since all the controls are also registered
    // to the IGraphics class which will draw them
    // which means the rendering order is kinda hard to controll
    g.DrawBitmap(mBitmap, mRECT, 1, &mBlend);
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    translate(dX, dY);
  }

  void translate(float dX, float dY) {
    for (int i = 0; i < mParameters->GetSize(); i++) {
      moveControl(mParameters->Get(i)->control, dX, dY);
    }
    moveControl(this, dX, dY);

    for (int i = 0; i < mInSocketsUi.GetSize(); i++) {
      moveControl(mInSocketsUi.Get(i), dX, dY);
      mInSockets->Get(i)->X += dX;
      mInSockets->Get(i)->Y += dY;
    }

    for (int i = 0; i < mOutSocketsUi.GetSize(); i++) {
      moveControl(mOutSocketsUi.Get(i), dX, dY);
      mOutSockets->Get(i)->X += dX;
      mOutSockets->Get(i)->Y += dY;
    }

    *X += dX;
    *Y += dY;
    mGraphics->SetAllControlsDirty();
  }

private:
  void moveControl(IControl* control, float x, float y) {
    if (control == nullptr) { return; }
    IRECT rect = control->GetRECT();
    rect.T += y;
    rect.L += x;
    rect.B += y;
    rect.R += x;
    control->SetTargetAndDrawRECTs(rect);
  }

protected:
  float* X;
  float* Y;
  IBitmap mBitmap;
  IBlend mBlend;
  IGraphics* mGraphics;
};
