#pragma once
#include "IControl.h"
#include "config.h"

typedef std::function<void(float x, float y, float scale)> backgroundCallback;

using namespace iplug;
using namespace igraphics;
class Background : public IControl {
public:
  Background(IGraphics* g, backgroundCallback pCallback) :
    IControl(IRECT(0, 0, PLUG_MAX_WIDTH, PLUG_MAX_HEIGHT), kNoParameter)
  {
    mBitmap = g->LoadBitmap(PNGBACKGROUND_FN, 1, false);
    mBlend = EBlend::Clobber;
    mGraphics = g;
    mY = 0;
    mX = 0;
    mGraphics->SetTranslation(0, 0);
    mCallback = pCallback;
    mScale = 1.0;
    windowX = 0;
    windowY = 0;
  }

  void Draw(IGraphics& g) override {
    g.DrawBitmap(mBitmap, GetRECT(), 1, &mBlend);
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    if (mod.L || mod.C) {
      mRECT.T += dY;
      mRECT.L += dX;
      mRECT.B += dY;
      mRECT.R += dX;
      SetTargetAndDrawRECTs(mRECT);
      SetDirty(true);
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
      windowX = bounds.R;
      windowY = bounds.B;
    }
  }

protected:
  IBitmap mBitmap;
  IBlend mBlend;
  IGraphics* mGraphics;
  float mX;
  float mY;
  int windowX;
  int windowY;
  float mScale;
  backgroundCallback mCallback;
};