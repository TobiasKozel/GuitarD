#pragma once

#include "IControl.h"

using namespace iplug;
using namespace igraphics;

class ScrollViewControl : public IControl {
  /** Settings */
  float mChildPaddingY = 10;
  float mDragThreshold = 4;
  bool mFullWidthChildren = false;
  bool mScrollBar = true;
  float mScrollBarWidth = 8;
  bool mDoCleanUp = true;

  /** Internal States */
  WDL_PtrList<IControl> mChildren;
  /** Scroll offset in Y */
  float mScrollY = 0;
  /** Content dimensions */
  float mContentHeight = 0;
  /** Content dimensions */
  float mContentWidth = 0;
  /** Whether the scrollbar is being dragged */
  bool mScrollBarDragging = false;
  bool mScrollBarHover = false;
  /** The size ratio of the scrollbar ( 1.0 is mRECT.H() )*/
  float mScrollBarRatio = 1;
  /** The child IControl the cursor is currently over */
  IControl* mMouseOver = nullptr;
  /** Distance the cursor was dragged, to prevent click events if it's over mDragThreshold */
  float mDistanceDragged = -1;
  /** Whether the control is already attached to IGraphics */
  bool mAttached = false;

  IColor mScrollBarColor = IColor(200, 255, 255, 255);
  IColor mScrollBarHoverColor = IColor(255, 255, 255, 255);
  IColor mScrollBarActiveColor = Theme::Colors::ACCENT;
public:
  ScrollViewControl(const IRECT bounds) : IControl(bounds) {}

  ScrollViewControl() : IControl({}) {}

  void appendChild(IControl* child) {
    mChildren.Add(child);
    if (mAttached) {
      child->SetDelegate(*GetDelegate());
    }
    mDirty = true;
  }

  void appendChild(IControl& child) {
    appendChild(&child);
  }

  void OnInit() override {
    IControl::OnInit();
    for (int i = 0; i < mChildren.GetSize(); i++) {
      IControl* c = mChildren.Get(i);
      c->SetDelegate(*GetDelegate());
    }
    mAttached = true;
  }

  void OnResize() override {
    mContentHeight = 0;
    mContentWidth = 0;
    const int childCount = mChildren.GetSize();
    if (childCount == 0) { return; }
    for (int i = 0; i < childCount; i++) {
      const bool isLast = i == childCount - 1;
      IControl* c = mChildren.Get(i);
      IRECT r = c->GetRECT();
      const float height = r.H();
      const float width = mFullWidthChildren ? mRECT.W() : r.W();
      if (width > mContentWidth) { mContentWidth = width; }
      r.L = r.T = 0;
      r.R = width;
      r.B = height;
      shiftRectY(r, mContentHeight);
      mContentHeight += height + (isLast ? 0.f : mChildPaddingY);
      c->SetTargetRECT(r); // This won't call OnResize() yet
      if (isLast) {
        const float scrollLimit = mRECT.H();
        if (r.B - mScrollY < scrollLimit) {
          mScrollY = r.B - scrollLimit;
        }
      }
    }
    if (mScrollY < 0) {
      mScrollY = 0;
    }
    for (int i = 0; i < mChildren.GetSize(); i++) {
      IControl* c = mChildren.Get(i);
      IRECT r = c->GetTargetRECT();
      shiftRectY(r, -mScrollY);
      r.Translate(mRECT.L, mRECT.T); // Apply the absolute position of the view itself
      c->SetTargetAndDrawRECTs(r); // Will call OnResize() on the child element
    }
    // Once all the children know the dimensions ask for a redraw
    mDirty = true;
  }

  bool removeChild(IControl& child, const bool wantsDelete = false) {
    const int index = mChildren.Find(&child);
    if (index != -1) {
      mChildren.Delete(index, wantsDelete);
      mDirty = true;
      return true;
    }
    return false;
  }

  void setChildPadding(const float padding) {
    mChildPaddingY = padding;
    OnResize();
  }

  /** Forces the children to be the same width as the ScrollView */
  void setFullWidthChildren(const bool full) {
    mFullWidthChildren = full;
    OnResize();
  }

  void setScrollBarEnable(const bool enable) {
    mScrollBar = enable;
    OnResize();
  }

  void setCleanUpEnabled(const bool enable) {
    mDoCleanUp = enable;
  }

  /** Scrolls in the y direction */
  void scroll(const float y) {
    mScrollY += y;
    OnResize();
  }

  void Draw(IGraphics& g) override {
    g.FillRect(COLOR_DARK_GRAY, mRECT);
    for (int i = 0; i < mChildren.GetSize(); i++) {
      mChildren.Get(i)->Draw(g);
    }
    if (mScrollBar) {
      IRECT scroll = mRECT.GetFromRight(mScrollBarWidth);
      const float height = mRECT.H();
      mScrollBarRatio = std::min(height / (mContentHeight + 1.f), 1.f);
      if (mScrollBarRatio < 1.f) {
        scroll.T += mScrollY * mScrollBarRatio;
        scroll.B = scroll.T + mScrollBarRatio * height;
        if (mScrollBarDragging) {
          g.FillRect(mScrollBarActiveColor, scroll);
        }
        else if (mScrollBarHover) {
          g.FillRect(mScrollBarHoverColor, scroll);
        }
        else {
          g.FillRect(mScrollBarColor, scroll);
        }
      }
    }
  }


  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override {
    if (mod.C) {
      IControl* c = getChildAtCoord(x, y);
      if (c != nullptr) {
        c->OnMouseWheel(x, y, mod, d);
        mDirty = true;
      }
    }
    else {
      scroll(d * -40);
    }
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    if (mScrollBarDragging) {
      scroll(dY / mScrollBarRatio);
    }
    else {
      if (mod.C) {
        IControl* c = getChildAtCoord(x, y);
        if (c != nullptr) {
          /** TODO: figure out a way to allow controls to handle a drag */
          c->OnMouseDrag(x, y, dX, dY, mod);
          mDirty = true;
        }
      }
      else {
        scroll(-dY);
        mDistanceDragged += abs(dX) + abs(dY);
      }
    }
  }

  IControl* getChildAtCoord(const float x, const float y) const {
    const IRECT click = { x, y, x, y };
    for (int i = 0; i < mChildren.GetSize(); i++) {
      IControl* c = mChildren.Get(i);
      IRECT r = c->GetTargetRECT();
      if (r.Contains(click)) {
        return c;
      }
    }
    return nullptr;
  }

  void OnMouseDblClick(float x, float y, const IMouseMod& mod) override {
    IControl* target = getChildAtCoord(x, y);
    if (target != nullptr) {
      target->OnMouseDblClick(x, y, mod);
      OnResize();
    }
  }

  void OnMouseDown(float x, float y, const IMouseMod& mod) override {
    if (mScrollBar && mRECT.GetFromRight(mScrollBarWidth).Contains(IRECT(x, y, x, y))) {
      mScrollBarDragging = true;
      mDirty = true;
    }
    else {
      IControl* target = getChildAtCoord(x, y);
      if (target != nullptr) {
        target->OnMouseDown(x, y, mod);
        OnResize();
      }
    }
  }

  void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    if (mDistanceDragged < mDragThreshold && !mScrollBarDragging) {
      IControl* target = getChildAtCoord(x, y);
      if (target != nullptr) {
        target->OnMouseUp(x, y, mod);
        OnResize();
      }
    }
    mScrollBarDragging = false;
    mDistanceDragged = 0;
    mDirty = true;
  }

  void OnMouseOver(float x, float y, const IMouseMod& mod) override {
    IControl::OnMouseOver(x, y, mod);
    if (mScrollBar) {
      const bool prevHover = mScrollBarHover;
      mScrollBarHover = mRECT.GetFromRight(mScrollBarWidth).Contains(IRECT(x, y, x, y));
      if (prevHover != mScrollBarHover) {
        mDirty = true;
      }
    }
    IControl* target = getChildAtCoord(x, y);
    if (mMouseOver != target && mMouseOver != nullptr) {
      mMouseOver->OnMouseOut();
    }
    if (target != nullptr) {
      target->OnMouseOver(x, y, mod);
      /** We'll just assume a child control doesn't change dimensions when hovering */
      mDirty = true;
    }
    mMouseOver = target;
  }

  void OnMouseOut() override {
    IControl::OnMouseOut();
    mScrollBarHover = false;
    if (mMouseOver != nullptr) {
      mMouseOver->OnMouseOut();
      mMouseOver = nullptr;
    }
    mDirty = true;
  }

  //virtual bool OnKeyDown(float x, float y, const IKeyPress& key) { return false; }

  //virtual bool OnKeyUp(float x, float y, const IKeyPress& key) { return false; }

  ~ScrollViewControl() {
    if (mDoCleanUp) {
      mChildren.Empty(true);
    }
  }

private:
  static void shiftRectY(IRECT& r, const float y) {
    r.T += y;
    r.B += y;
  }
};
