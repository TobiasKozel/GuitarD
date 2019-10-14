#pragma once
#include "IControl.h"

using namespace iplug;
using namespace igraphics;

typedef std::function<void(float x, float y)> nodeBackgroundCallback;

/**
 * This class represents a Node on the UI, it's seperate to the node itself
 * since it will only exists as long as the UI window is open but is owned by the node
 */
class NodeBackground : public IControl {
public:
  NodeBackground(IGraphics* g, const char* bg, float L, float T, nodeBackgroundCallback pCallback) :
    IControl(IRECT(L, T, L, T), kNoParameter)
  {
    mBitmap = g->LoadBitmap(bg, 1, false);
    mRECT.R = L + mBitmap.W();
    mRECT.B = T + mBitmap.H();
    SetTargetAndDrawRECTs(mRECT);
    mBlend = EBlend::Clobber;
    mGraphics = g;
    mCallback = pCallback;
  }


  void Draw(IGraphics& g) override {
    g.DrawBitmap(mBitmap, GetRECT(), 1, &mBlend);
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    mCallback(dX, dY);
    mGraphics->SetAllControlsDirty();
  }
protected:
  IBitmap mBitmap;
  IBlend mBlend;
  IGraphics* mGraphics;
  nodeBackgroundCallback mCallback;
};
