#pragma once
#include "IControl.h"
#include "src/graph/misc/ParameterCoupling.h"
#include "src/graph/misc/NodeSocket.h"
#include "src/graph/ui/NodeSocketUi.h"
#include "src/graph/misc/MessageBus.h"
#include "src/graph/misc/GStructs.h"

class Node;

struct NodeUiParam {
  iplug::igraphics::IGraphics* pGraphics;
  const char* pBg;
  float* X;
  float* Y;
  bool* byPassed;
  WDL_PtrList<ParameterCoupling>* pParameters;
  WDL_PtrList<NodeSocket>* inSockets;
  WDL_PtrList<NodeSocket>* outSockets;
  Node* node;
};

using namespace iplug;
using namespace igraphics;


/**
 * This class represents a Node on the UI, it's seperate to the node itself
 * since it will only exists as long as the UI window is open but is owned by the node
 */
class NodeUi : public IControl {
  MessageBus::Subscription<NodeSocket*> mNodeConnectBetweenEvent;
  IRECT mCloseButton;
  IRECT mDisconnectAllButton;
  IRECT mByPassButton;
  WDL_PtrList<ParameterCoupling>* mParameters;
  WDL_PtrList<NodeSocket>* mInSockets;
  WDL_PtrList<NodeSocket>* mOutSockets;
  WDL_PtrList<NodeSocketUi> mInSocketsUi;
  WDL_PtrList<NodeSocketUi> mOutSocketsUi;
  Node* mParentNode;
public:
  NodeUi(NodeUiParam pParam) :
    IControl(IRECT(0, 0, 0, 0), kNoParameter)
  {
    mDragging = false;
    X = pParam.X;
    Y = pParam.Y;
    mGraphics = pParam.pGraphics;
    mParameters = pParam.pParameters;
    mInSockets = pParam.inSockets;
    mOutSockets = pParam.outSockets;
    mParentNode = pParam.node;
    mBypassed = pParam.byPassed;

    mBitmap = mGraphics->LoadBitmap(pParam.pBg, 1, false);
    float w = mBitmap.W();
    float h = mBitmap.H();
    IRECT rect;
    rect.L = *X - w / 2;
    rect.R = *X + w / 2;
    rect.T = *Y - h / 2;
    rect.B = *Y + h / 2;
    SetTargetAndDrawRECTs(rect);
#define buttonX 40
#define buttonY 0
#define buttonW 40
#define buttonH 40
    mCloseButton.L = rect.R - buttonX;
    mCloseButton.T = rect.T + buttonY;
    mCloseButton.R = mCloseButton.L + buttonW;
    mCloseButton.B = mCloseButton.T + buttonH;

    mDisconnectAllButton.L = rect.R - buttonW - buttonX;
    mDisconnectAllButton.T = rect.T + buttonY;
    mDisconnectAllButton.R = mDisconnectAllButton.L + buttonW;
    mDisconnectAllButton.B = mDisconnectAllButton.T + buttonH;

    mByPassButton.L = rect.L;
    mByPassButton.T = rect.T + buttonY;
    mByPassButton.R = mByPassButton.L + buttonW;
    mByPassButton.B = mByPassButton.T + buttonH;

    mBlend = EBlend::Clobber;

    mNodeConnectBetweenEvent.subscribe("NodeSpliceIn", [&](NodeSocket* socket) {
      NodeSocket* prev = mInSockets->Get(0);
      NodeSocket* next = mOutSockets->Get(0);
      if (prev != nullptr && next != nullptr) {
        prev->connect(socket->connectedTo);
        next->connect(socket);
      }
    });
  }

  ~NodeUi() {
  }

  void setUp() {
    for (int i = 0; i < mParameters->GetSize(); i++) {
      ParameterCoupling* couple = mParameters->Get(i);
      double value = *(couple->value);
      float px = *X + couple->x - (couple->w * 0.5f);
      float py = *Y + couple->y - (couple->h * 0.5f);
      IRECT controlPos(px, py, px + couple->w, py + couple->h);
      // use the daw parameter to sync the values if possible
      if (couple->parameterIdx != kNoParameter) {
        couple->parameter->Set(value);
        couple->control = new IVKnobControl(
          controlPos, couple->parameterIdx
        );
      }
      else {
        // use the callback to get tha value to the dsp, won't allow automation though
        couple->control = new IVKnobControl(
          controlPos, [couple](IControl* pCaller) {
            *(couple->value) =
              (pCaller->GetValue() * (couple->max - couple->min)) + couple->min;
          }
        );
      }
      mGraphics->AttachControl(couple->control);
      couple->control->SetValue(
        (value - couple->min) / (couple->max - couple->min)
      );

      // optinally hide the lables etc
      //IVectorBase* vcontrol = dynamic_cast<IVectorBase*>(couple->control);
      //if (vcontrol != nullptr) {
      //  vcontrol->SetShowLabel(false);
      //  vcontrol->SetShowValue(false);
      //}
    }

    for (int i = 0; i < mInSockets->GetSize(); i++) {
      NodeSocketUi* socket = new NodeSocketUi(mGraphics, mInSockets->Get(i));
      mGraphics->AttachControl(socket);
      mInSocketsUi.Add(socket);
    }

    for (int i = 0; i < mOutSockets->GetSize(); i++) {
      NodeSocketUi* socket = new NodeSocketUi(mGraphics, mOutSockets->Get(i));
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
    //g.FillRect(IColor(255, 10, 10, 10), mRECT);
    g.DrawRect(IColor(255, 0, 255, 0), mCloseButton);
    g.DrawRect(IColor(255, 0, 255, 0), mDisconnectAllButton);
    g.DrawRect(IColor(255, 255, *mBypassed ? 0 : 255, 0), mByPassButton);
  }

  void OnMouseUp(float x, float y, const IMouseMod& mod) {
    if (mDragging) {
      mDragging = false;
      MessageBus::fireEvent<Coord2d>("NodeDraggedEnd", Coord2d{ x, y });
      return;
    }
    if (mCloseButton.Contains(IRECT(x, y, x, y))) {
      MessageBus::fireEvent<Node*>("NodeDeleted", mParentNode);
    }
    if (mDisconnectAllButton.Contains(IRECT(x, y, x, y))) {
      MessageBus::fireEvent<Node*>("NodeDisconnectAll", mParentNode);
    }
    if (mByPassButton.Contains(IRECT(x, y, x, y))) {
      *mBypassed = !*mBypassed;
      mDirty = true;
    }
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    mDragging = true;
    MessageBus::fireEvent<Coord2d>("NodeDragged", Coord2d {x, y});
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

    mCloseButton.Translate(dX, dY);
    mDisconnectAllButton.Translate(dX, dY);
    mByPassButton.Translate(dX, dY);

    mGraphics->SetAllControlsDirty();
  }

  void setTranslation(float x, float y) {
    float dX = x - *X;
    float dY = y - *Y;
    translate(dX, dY);
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
  bool mDragging;
  float* X;
  float* Y;
  bool* mBypassed;
  IBitmap mBitmap;
  IBlend mBlend;
  IGraphics* mGraphics;
};
