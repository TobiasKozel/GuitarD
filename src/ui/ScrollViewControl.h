#pragma once

#include "IControl.h"

using namespace iplug;
using namespace igraphics;

class ScrollViewControl : public IControl {
  WDL_PtrList<IControl> mChildren;
  float mChildPaddingY = 10;
  float mScrollY = 0;
  float mContentHeight = 0;
  float mContentWidth = 0;
public:
  ScrollViewControl(IRECT bounds) : IControl(bounds) {

  }

  void appendChild(IControl* child) {
    mChildren.Add(child);
  }

  void appendChild(IControl& child) {
    appendChild(&child);
  }

  bool removeChild(IControl& child, const bool wantsDelete = false) {
    int index = mChildren.Find(&child);
    if (index != -1) {
      mChildren.Delete(index, wantsDelete);
      return true;
    }
    return false;
  }

  static void shiftRectY(IRECT& r, const float y) {
    r.T += y;
    r.B += y;
  }

  void layout() {
    mContentHeight = 0;
    mContentWidth = 0;
    const int childCount = mChildren.GetSize();
    if (childCount == 0) { return; }
    for (int i = 0; i < childCount; i++) {
      const bool isLast = i == childCount - 1;
      IControl* c = mChildren.Get(i);
      IRECT r = c->GetTargetRECT();
      const float height = r.H();
      const float width = r.W();
      if (width > mContentWidth) { mContentWidth = width; }
      r.L = r.T = 0;
      r.R = width;
      r.B = height;
      shiftRectY(r, mContentHeight);
      mContentHeight += height + (isLast ? 0.f : mChildPaddingY);
      c->SetTargetRECT(r);
      if (isLast) {
        const float scrollLimit = mRECT.H() * 0.7;
        if (r.B - mScrollY < scrollLimit) {
          mScrollY = r.B - scrollLimit;
        }
      }
    }
    if (mContentHeight > mChildPaddingY) {
      mContentHeight -= mChildPaddingY;
    }
    if (mScrollY < 0) {
      mScrollY = 0;
    }
    for (int i = 0; i < mChildren.GetSize(); i++) {
      IControl* c = mChildren.Get(i);
      IRECT r = c->GetTargetRECT();
      shiftRectY(r, -mScrollY);
      r.Translate(mRECT.L, mRECT.T);
      c->SetTargetAndDrawRECTs(r);
    }
  }

  void Draw(IGraphics& g) override {
    layout();
    g.FillRect(COLOR_DARK_GRAY, mTargetRECT);
    for (int i = 0; i < mChildren.GetSize(); i++) {
      mChildren.Get(i)->Draw(g);
    }
  }


  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override {
    mScrollY -= d * 20;
    mDirty = true;
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    mScrollY -= dY;
    mDirty = true;
  }

#define RELAY_MOUSE_EVENT(function) IRECT click = { x, y, x, y };\
  for (int i = 0; i < mChildren.GetSize(); i++) {\
    IControl* c = mChildren.Get(i);\
    if (c->GetTargetRECT().Contains(click)) {\
      c->function(x, y, mod);\
      mDirty = true;\
      return;\
    }\
  }\

  void OnMouseDblClick(float x, float y, const IMouseMod& mod) override { RELAY_MOUSE_EVENT(OnMouseDblClick) }

  void OnMouseDown(float x, float y, const IMouseMod& mod) override { RELAY_MOUSE_EVENT(OnMouseDown) }

  void OnMouseUp(float x, float y, const IMouseMod& mod) override { RELAY_MOUSE_EVENT(OnMouseUp) }

  ~ScrollViewControl() {
    mChildren.Empty(true);
  }
};
