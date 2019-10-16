#pragma once
#include "IControl.h"

using namespace iplug;
using namespace igraphics;

typedef std::function<void(int connectedTo)> nodeSocketCallback;

class NodeSocketUi : public IControl {
public:
  NodeSocketUi(IGraphics* g, const char* bitmap, float L, float T, int index, bool out, nodeSocketCallback pCallback) :
    IControl(IRECT(L, T, L, T), kNoParameter)
  {
    //mBitmap = g->LoadBitmap(bitmap, 1, false);
    //mRECT.R = L + mBitmap.W();
    //mRECT.B = T + mBitmap.H();
    mIndex = index;
    mOut = out;
    mDiameter = 30;
    mRadius = mDiameter * 0.5;
    mRECT.R = L + mDiameter;
    mRECT.B = T + mDiameter;
    SetTargetAndDrawRECTs(mRECT);
    mBlend = EBlend::Clobber;
    mGraphics = g;
    mCallback = pCallback;
    color.A = 255;
    if (out) {
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
    mStartX = x;
    mStartY = y;
  }

  virtual void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    mDragging = false;
    SetRECT(mTargetRECT);
    mGraphics->SetAllControlsDirty();
    // mCallback(dX, dY);
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    mDragging = true;
    SetRECT(IRECT(0, 0, 2000, 2000));
    mCurrentX = x;
    mCurrentY = y;
    mGraphics->SetAllControlsDirty();
  }

protected:
  IBitmap mBitmap;
  IBlend mBlend;
  IGraphics* mGraphics;
  nodeSocketCallback mCallback;
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