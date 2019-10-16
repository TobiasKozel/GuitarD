#pragma once
#include "IControl.h"
#include "config.h"

typedef std::function<void(float x, float y, float scale)> backgroundCallback;

using namespace iplug;
using namespace igraphics;
class GraphBackground : public IControl {
public:
  GraphBackground(IGraphics* g, backgroundCallback pCallback) :
    IControl(IRECT(0, 0, g->Width(), g->Height()), kNoParameter)
  {
    mBitmap = g->LoadBitmap(PNGBACKGROUND_FN, 1, false);
    mBlend = EBlend::Clobber;
    mGraphics = g;
    mY = mX = 0;
    mCallback = pCallback;
    mScale = 1.0;
    offsetX = offsetY = windowY = windowX = 0;
  }

  void Draw(IGraphics& g) override {
    g.FillRect(COLOR_GRAY, mRECT);
    int x = offsetX % windowX;
    int y = offsetY % windowY;
    g.DrawBitmap(mBitmap, IRECT(x, y, windowX, windowY), 1, &mBlend);
    g.DrawBitmap(mBitmap, IRECT(x - windowX, y, windowX, windowY), 1, &mBlend);
    g.DrawBitmap(mBitmap, IRECT(x, y - windowY, windowX, windowY), 1, &mBlend);
    g.DrawBitmap(mBitmap, IRECT(x - windowX, y - windowY, windowX, windowY), 1, &mBlend);
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    if (mod.L || mod.C) {
      offsetX += dX;
      offsetY += dY;
      mGraphics->SetAllControlsDirty();
      mCallback(dX, dY, 1.f);
    }
  }

  void OnMouseDown(float x, float y, const IMouseMod& mod) {
    if (mod.R) {
      // prolly open the menu to add nodes
    }
  }



  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override {
    // mRECT.Translate(-x, -y);
    // mRECT.Scale(1 - d / 10.f);
    // mRECT.Translate(x, y);
    mScale += d / 20.0;
    WDBGMSG("scale %f \n", mScale);
    if (mScale > 0.3 && mScale < 2) {
      // TODO sucks hard, at least some kind of scaling
      mGraphics->Resize(windowX, windowY, mScale);
    }
    //mCallback(mX, mY, mScale);
  }

  void OnResize() override {
    if (mGraphics != nullptr) {
      IRECT bounds = mGraphics->GetBounds();
      // keep track of the window size so the background alsoways fills the screen
      //bounds.R += 200;
      //bounds.B += 200;
      windowX = bounds.R;
      windowY = bounds.B;
      mRECT = bounds;
      mTargetRECT = bounds;
    }
  }

protected:
  IBitmap mBitmap;
  IBlend mBlend;
  IGraphics* mGraphics;
  float mX;
  float mY;
  int offsetX;
  int offsetY;
  int windowX;
  int windowY;
  float mScale;
  backgroundCallback mCallback;
};