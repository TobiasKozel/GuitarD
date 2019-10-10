#pragma once
#include "IControl.h"
#include "config.h"

using namespace iplug;
using namespace igraphics;
class Background : public IControl {
public:
  Background(IGraphics* g) :
    IControl(IRECT(0, 0, PLUG_MAX_WIDTH, PLUG_MAX_HEIGHT), kNoParameter) {
    mBitmap = g->LoadBitmap(PNGBACKGROUND_FN, 1, false);
    mBlend = EBlend::Clobber;
  }

  void Draw(IGraphics& g) override {
    g.DrawBitmap(mBitmap, GetRECT(), 1, &mBlend);
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    WDBGMSG("x %f y %f\n", x, y);
    mRECT.T += dY;
    mRECT.L += dX;
    SetDirty(true);
  }

protected:
  IBitmap mBitmap;
  IBlend mBlend;
};