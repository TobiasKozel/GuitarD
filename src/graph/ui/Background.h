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
    mScale = 1;
    mCallback = pCallback;
  }

  void Draw(IGraphics& g) override {
    g.DrawBitmap(mBitmap, GetRECT(), 1, &mBlend);
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    mRECT.T += dY;
    mRECT.L += dX;
    SetDirty(true);
    mCallback(dX, dY, mScale);
  }

  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override {
    // mRECT.Translate(-x, -y);
    // mRECT.Scale(1 - d / 10.f);
    // mRECT.Translate(x, y);
    mScale += d;
    WDBGMSG("d %f x %f \n", d, x);
    mCallback(mX, mY, mScale);
    SetDirty(true);
  }

protected:
  IBitmap mBitmap;
  IBlend mBlend;
  IGraphics* mGraphics;
  float mX;
  float mY;
  float mScale;
  backgroundCallback mCallback;
};