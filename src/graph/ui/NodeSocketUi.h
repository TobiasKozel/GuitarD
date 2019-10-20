#pragma once
#include "IControl.h"
#include "src/constants.h"
#include "src/graph/misc/NodeSocket.h"

using namespace iplug;
using namespace igraphics;

typedef std::function<void(NodeSocket* connectedTo, int ownIndex)> NodeSocketCallback;

class NodeSocketUi : public IControl {
public:
  NodeSocketUi(IGraphics* g, NodeSocket* socket, NodeSocketCallback callback) :
    IControl(IRECT(0, 0, 0, 0), kNoParameter)
  {
    //mBitmap = g->LoadBitmap(bitmap, 1, false);
    //mRECT.R = L + mBitmap.W();
    //mRECT.B = T + mBitmap.H();
    mSocket = socket;
    mDiameter = SOCKETDIAMETER;
    mRadius = mDiameter * 0.5;
    mRECT.L = socket->X;
    mRECT.T = socket->Y;
    mRECT.R = mRECT.L + mDiameter;
    mRECT.B = mRECT.T + mDiameter;
    SetTargetAndDrawRECTs(mRECT);
    mBlend = EBlend::Clobber;
    mGraphics = g;
    mCallback = callback;
    color.A = 255;
    if (socket->isInput) {
      color.R = 255;
    }
    else {
      color.B = 255;
    }
    
  }

  void Draw(IGraphics& g) override {
    // g.DrawBitmap(mBitmap, GetRECT(), 1, &mBlend);
    g.DrawCircle(color, mTargetRECT.L + mRadius, mTargetRECT.T + mRadius, mRadius, &mBlend, 10);
    if (mDragging) {

      g.DrawLine(color, mStartX, mStartY, mCurrentX, mCurrentY, &mBlend, 5);
    }
  }

  void OnMouseDown(float x, float y, const IMouseMod& mod) override {
    auto center = mRECT;
    center.ScaleAboutCentre(0);
    mStartX = center.L;
    mStartY = center.T;
  }

  virtual void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    mDragging = false;
    SetRECT(mTargetRECT);
    mGraphics->SetAllControlsDirty();
    IControl* target = mGraphics->GetControl(
      mGraphics->GetMouseControlIdx(x, y, false)
    );
    if (target != nullptr) {
      NodeSocketUi* targetUi = dynamic_cast<NodeSocketUi*>(target);
      if (targetUi != nullptr) {
        if (!targetUi->mSocket->isInput && mSocket->isInput) {
          mCallback(targetUi->mSocket, mSocket->ownIndex);
        }
      }
    }
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    mDragging = true;
    SetRECT(IRECT(0, 0, 2000, 2000));
    mCurrentX = x;
    mCurrentY = y;
    mGraphics->SetAllControlsDirty();
  }

  /**
   * Disconnect the input on double click
   */
  void OnMouseDblClick(float x, float y, const IMouseMod& mod) override {
    mCallback(nullptr, mSocket->ownIndex);
    mGraphics->SetAllControlsDirty();
  }

protected:
  NodeSocket* mSocket;
  IBitmap mBitmap;
  IBlend mBlend;
  IGraphics* mGraphics;
  NodeSocketCallback mCallback;
  IColor color;
  float mDiameter;
  float mRadius;
  int mIndex;
  bool mOut;
  bool mDragging;
  float mStartX;
  float mStartY;
  float mCurrentY;
  float mCurrentX;

};