#pragma once
#include "IControl.h"
#include "src/ui/theme.h"
#include "src/misc/MessageBus.h"

using namespace iplug;
using namespace igraphics;

typedef std::function<void(float x, float y, float scale)> BackgroundMoveCallback;

class GraphBackground : public IControl {
public:
  GraphBackground(IGraphics* g, BackgroundMoveCallback pCallback) :
    IControl(IRECT(0, 0, g->Width(), g->Height()), kNoParameter)
  {
    mGraphics = g;
    mY = mX = 0;
    mCallback = pCallback;
    mScale = 1.0;
    offsetX = offsetY = 0;
    mColorBackground = IColor(255, COLORBACKGROUND);
    mColorBackgroundDetail = IColor(255, COLORBACKGROUNDDETAIL);
    storeSize();
    triggeredScale = false;
  }

  /**
   * Draw the simple background shapes
   */
  void Draw(IGraphics& g) override {
    int windowX = g.Width();
    int windowY = g.Height();
    g.FillRect(mColorBackground, mRECT);
    for (float y = fmod(offsetY, BACKGROUNDDETAILDIST) - BACKGROUNDDETAILDIST; y < windowY + BACKGROUNDDETAILDIST; y += BACKGROUNDDETAILDIST) {
      for (float x = fmod(offsetX, BACKGROUNDDETAILDIST) - BACKGROUNDDETAILDIST; x < windowX + BACKGROUNDDETAILDIST; x += BACKGROUNDDETAILDIST) {
        float x1 = x - (BACKGROUNDDETAILSIZE / 2);
        float y1 = y - (BACKGROUNDDETAILWIDTH / 2);
        g.FillRect(mColorBackgroundDetail, IRECT(
          x1, y1, x1 + BACKGROUNDDETAILSIZE, y1 + BACKGROUNDDETAILWIDTH
        ));
        x1 = x - (BACKGROUNDDETAILWIDTH / 2);
        y1 = y - (BACKGROUNDDETAILSIZE / 2);
        g.FillRect(mColorBackgroundDetail, IRECT(
          x1, y1, x1 + BACKGROUNDDETAILWIDTH, y1 + BACKGROUNDDETAILSIZE
        ));
      }
    }
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    if (mod.L || mod.C) {
      translate(dX, dY);
      mGraphics->SetAllControlsDirty();
    }
  }

  void OnMouseDown(float x, float y, const IMouseMod& mod) override {
    if (mod.R || mod.L) {
      MessageBus::fireEvent<bool>(MessageBus::OpenGallery, mod.R);
    }
  }

  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override {
    /**
     * TODOG sucks hard, at least some kind of scaling
     * We're essentially using the renderer to scale the graph (and everything else)
     * This also changes the window size, so here's some counter scaling
     * to keep it roughly the same. It does some resizeing because of rounding errors though.
     */
    float newScale = mScale + d / 20.f;
    float w = lastWidth;
    float h = lastHeight;
    if (newScale > 0.45 && newScale < 2) {
      if (w / newScale >= PLUG_MAX_WIDTH && h / newScale >= PLUG_MAX_HEIGHT) {
        return;
      }
      if (w / newScale <= PLUG_MIN_WIDTH && h / newScale <= PLUG_MIN_HEIGHT) {
        return;
      }

      lastWidth = ((w * mScale) / newScale);
      lastHeight = ((h * mScale) / newScale);

      float dX = ((x * mScale) / newScale) - x;
      float dY = ((y * mScale) / newScale) - y;


      triggeredScale = true;
      mGraphics->Resize(ceil(lastWidth), ceil(lastHeight), newScale);
      translate(dX, dY);
      mScale = newScale;
    }
  }

  void OnResize() override {
    if (mGraphics != nullptr) {
      if (!triggeredScale) {
        storeSize();
      }
      triggeredScale = false;
      IRECT bounds = mGraphics->GetBounds();
      mRECT = bounds;
      mTargetRECT = bounds;
    }
  }

  float mScale;

protected:
  void translate(float dX, float dY) {
    offsetX += (dX);
    offsetY += (dY);
    mCallback(dX, dY, 1.f);
  }

  void storeSize() {
    lastHeight = mGraphics->Height();
    lastWidth = mGraphics->Width();
  }

  IGraphics* mGraphics;
  float mX;
  float mY;
  float offsetX;
  float offsetY;
  float lastWidth;
  float lastHeight;
  bool triggeredScale;
  IColor mColorBackground;
  IColor mColorBackgroundDetail;
  BackgroundMoveCallback mCallback;
};
