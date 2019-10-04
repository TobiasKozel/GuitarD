#pragma once

#include "IControl.h"

using namespace iplug::igraphics;

class UiNode : public IControl, public IVectorBase {
  EVShape mShape;
public:
  UiNode(iplug::igraphics::IRECT bounds)
    : IControl(bounds, iplug::kNoParameter)
  {
    mShape = EVShape::Rectangle;
    mWidgetBounds = bounds;
  }
  void Draw(IGraphics& g)
  {
    DrawBackGround(g, mRECT);
    DrawWidget(g);
    DrawLabel(g);
    // DrawValue(g, false);
  }

  void DrawWidget(IGraphics& g)
  {
    // DrawHandle(g, mShape, mWidgetBounds, false, false);
  }

  void OnResize()
  {
    SetTargetRECT(MakeRects(mRECT, true));
    SetDirty(false);
  }

  bool IsHit(float x, float y) const
  {
    return mWidgetBounds.Contains(x, y);
  }

  void OnEndAnimation()
  {
    // SetValue(0.);
    // IControl::OnEndAnimation();
  }
};