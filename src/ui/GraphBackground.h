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
    offsetX = offsetY = windowY = windowX = 0;
    mColorBackground = IColor(255, COLORBACKGROUND);
    mColorBackgroundDetail = IColor(255, COLORBACKGROUNDDETAIL);
  }

  void Draw(IGraphics& g) override {
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
      offsetX += (dX);
      offsetY += (dY);
      mGraphics->SetAllControlsDirty();
      mCallback(dX, dY, 1.f);
    }
  }

  void OnMouseDown(float x, float y, const IMouseMod& mod) override {
    if (mod.R || mod.L) {
      MessageBus::fireEvent<bool>(MessageBus::OpenGallery, mod.R);
    }
  }

  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override {
    // mRECT.Translate(-x, -y);
    // mRECT.Scale(1 - d / 10.f);
    // mRECT.Translate(x, y);
    // WDBGMSG("scale %f \n", mScale);
    float newScale = mScale + d / 20.f;
    if (newScale > 0.3 && newScale < 2) {
      // TODOG sucks hard, at least some kind of scaling
      float w = mGraphics->Width();
      float h = mGraphics->Height();
      
      w = (float) (w * mScale);
      h = (float) (h * mScale);
      w = (int) (w / newScale);
      h = (int) (h / newScale);
      mGraphics->Resize(w, h, newScale);
      mScale = newScale;
    }
    //mCallback(mX, mY, mScale);
  }

  void OnResize() override {
    if (mGraphics != nullptr) {
      IRECT bounds = mGraphics->GetBounds();
      // keep track of the window size so the background alsoways fills the screen
      //bounds.R += 200;
      //bounds.B += 200;
      windowX = static_cast<int>(bounds.R);
      windowY = static_cast<int>(bounds.B);
      mRECT = bounds;
      mTargetRECT = bounds;
    }
  }

protected:
  IGraphics* mGraphics;
  float mX;
  float mY;
  float offsetX;
  float offsetY;
  int windowX;
  int windowY;
  float mScale;
  IColor mColorBackground;
  IColor mColorBackgroundDetail;
  BackgroundMoveCallback mCallback;
};
