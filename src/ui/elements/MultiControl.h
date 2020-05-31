#pragma

#include "IControl.h"
#include "../../types/GPointerList.h"
#include "../../types/GTypes.h"

namespace guitard {

  class MultiControlChild {
  public:
    bool mHandleDrag = true;
    bool mHandleScroll = true;
  };

  class GKnobControl : public MultiControlChild, public IVKnobControl {
  public:
    GKnobControl(IRECT r, int p, const char* n) : IVKnobControl(r, p, n)
    {
    }

    //void Draw(IGraphics& g) override {
    //  IVKnobControl::Draw(g);
    //};
  };

  class MultiControl : public IControl {
    /** All the controls managed by it */
    PointerList<IControl> mChildren;

    /** Whether the control is already attached to IGraphics */
    bool mAttached = false;

    bool mDoCleanUp = true;

    /** The child IControl the cursor is currently over */
    IControl* mMouseOver = nullptr;

    ILayerPtr mLayer;
  public:
    MultiControl(const IRECT bounds) : IControl(bounds) {}

    MultiControl() : IControl({}) {}

    ~MultiControl() {
      if (mDoCleanUp) {
        mChildren.clear(true);
      }
    }

    /**
     * Child handling
     */
    void appendChild(IControl* child) {
      if (mChildren.find(child) != -1) { return; } // No duplicates
      for (int i = mChildren.size() - 1; i > 0; i--) {
        if (mChildren[i]->GetZIndex() >= child->GetZIndex()) {
          mChildren.insert(child, i);
          if (mAttached) {
            child->SetDelegate(*GetDelegate());
          }
          OnResize();
          return;
        }
      }
    }

    void appendChild(IControl& child) {
      appendChild(&child);
    }

    void removeChild(IControl* child, const bool wantsDelete = false, bool skipResize = false) {
      const int index = mChildren.find(child);
      if (index != -1) {
        mChildren[index]->OnDetached();
        mChildren.remove(index, wantsDelete);
        if (!skipResize) {
          OnResize();
        }
      }
    }

    void removeChild(IControl& child, const bool wantsDelete = false) {
      removeChild(&child, wantsDelete);
    }

    void clearChildren(bool wantsDelete = false) {
      while (mChildren.size()) {
        removeChild(mChildren[0], wantsDelete, true);
      }
      OnResize();
    }

    IControl* getChildAtCoord(const float x, const float y) const {
      const IRECT click = { x, y, x, y };
      for (int i = 0; i < mChildren.size(); i++) {
        IControl* c = mChildren[i];
        if (c->IsHidden()) { continue; }
        IRECT r = c->GetTargetRECT();
        if (r.Contains(click)) {
          return c;
        }
      }
      return nullptr;
    }

    /**
     * Overrides from base class
     */
    void OnInit() override {
      IControl::OnInit();
      for (int i = 0; i < mChildren.size(); i++) {
        IControl* c = mChildren[i];
        c->SetDelegate(*GetDelegate());
      }
      mAttached = true;
    }

    void Draw(IGraphics& g) override {
      g.StartLayer(this, mRECT);
      for (int i = 0; i < mChildren.size(); i++) {
        if (!mChildren[i]->IsHidden()) {
          mChildren[i]->Draw(g);
        }
      }
      mLayer = g.EndLayer();
      g.DrawFittedLayer(mLayer, mRECT, &mBlend);
      /**
       * Will always redraw for now
       */
      mDirty = true;
    }

    void OnResize() override {
      if (!mAttached) { return; }

    }

    /**
     * Event handling and propagation to child controls
     */
    void OnMouseDblClick(float x, float y, const IMouseMod& mod) override {
      IControl* target = getChildAtCoord(x, y);
      if (target != nullptr) {
        target->OnMouseDblClick(x, y, mod);
        OnResize();
      }
    }

    void OnMouseDown(float x, float y, const IMouseMod& mod) override {
      IControl* target = getChildAtCoord(x, y);
      if (target != nullptr) {
        target->OnMouseDown(x, y, mod);
        OnResize();
      }
    }

    void OnMouseUp(float x, float y, const IMouseMod& mod) override {
      //if (mDistanceDragged < mDragThreshold && !mScrollBarDragging) {
      //  IControl* target = getChildAtCoord(x, y);
      //  if (target != nullptr) {
      //    target->OnMouseUp(x, y, mod);
      //    OnResize();
      //  }
      //}
      //mScrollBarDragging = false;
      //mDistanceDragged = 0;
      //mCapturedControl = nullptr;
      //mDirty = true;
      //mCapturedControl = nullptr;
    }

    void OnMouseOver(float x, float y, const IMouseMod& mod) override {
      //IControl::OnMouseOver(x, y, mod);
      //if (mScrollBar) {
      //  const bool prevHover = mScrollBarHover;
      //  mScrollBarHover = mRECT.GetFromRight(mScrollBarWidth).Contains(IRECT(x, y, x, y));
      //  if (prevHover != mScrollBarHover) {
      //    mDirty = true;
      //  }
      //}
      //IControl* target = getChildAtCoord(x, y);
      //if (mMouseOver != target && mMouseOver != nullptr) {
      //  mMouseOver->OnMouseOut();
      //}
      //if (target != nullptr) {
      //  target->OnMouseOver(x, y, mod);
      //  /** We'll just assume a child control doesn't change dimensions when hovering */
      //  mDirty = true;
      //}
      //mMouseOver = target;
    }

    void OnMouseOut() override {
      //IControl::OnMouseOut();
      //if (mMouseOver != nullptr) { // Relay mouse out to child
      //  mMouseOver->OnMouseOut();
      //  mMouseOver = nullptr;
      //}
      //mDirty = true;
    }

    //virtual bool OnKeyDown(float x, float y, const IKeyPress& key) { return false; }

    //virtual bool OnKeyUp(float x, float y, con
  };
}