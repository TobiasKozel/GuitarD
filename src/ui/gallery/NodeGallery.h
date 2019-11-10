#pragma once
#include "IControl.h"
#include "src/node/Node.h"
#include "src/misc/MessageBus.h"
#include "NodeGalleryCategory.h"
#include "src/ui/theme.h"
#include "src/graph/GraphStats.h"

using namespace iplug;
using namespace igraphics;


class NodeGallery : public IControl {
public:
  NodeGallery(IGraphics* g) :
    IControl(IRECT(), kNoParameter)
  {
    mAccentColor = IColor(255, COLORACCENT);
    mBackgroundColor = IColor(255, GALLERYBACKGROUND);
    mAddSignColor = IColor(255, COLORBACKGROUND);
    mPadding = 10;
    mGraphics = g;
    init();
    mIsOpen = false;
    mDragging = false;
    OnResize();
    mOpenGalleryEvent.subscribe(MessageBus::OpenGallery, [&](bool open) {
      this->openGallery(open);
    });
    mStats = DEBUGFONT;
  }

  ~NodeGallery() {
    mCategories.Empty(true);
  }

  void openGallery(bool open = true) {
    if (open == mIsOpen) { return; }
    mIsOpen = open;
    OnResize();
    if (mIsOpen) {
      // ensure it's on top of the drawing stack
      mGraphics->RemoveControl(this);
      mGraphics->AttachControl(this);
    }
    else {
      // redraw the whole screen
      mGraphics->SetAllControlsDirty();
    }
  }

  void Draw(IGraphics& g) override {
    if (mIsOpen) {
      g.FillRect(mBackgroundColor, mRECT);
      for (int i = 0; i < mCategories.GetSize(); i++) {
        mCategories.Get(i)->Draw(g);
      }
      g.FillRect(mBackgroundColor, IRECT(mRECT.L, mRECT.T - 1, mRECT.R, mRECT.T + GALLERYPADDING));
      g.FillRect(mBackgroundColor, IRECT(mRECT.L, mRECT.B - GALLERYPADDING, mRECT.R, mRECT.B + 1));
    }
    else {
      g.FillCircle(mAccentColor, mRECT);
      float x1 = mRECT.L + GALLERYADDCIRCLERADIUS - (GALLERYADDSIGNSIZE / 2);
      float y1 = mRECT.T + GALLERYADDCIRCLERADIUS - (GALLERYADDSIGNWIDTH / 2);
      g.FillRect(mAddSignColor, IRECT(
        x1, y1, x1 + GALLERYADDSIGNSIZE, y1 + GALLERYADDSIGNWIDTH
      ));
      x1 = mRECT.L + GALLERYADDCIRCLERADIUS - (GALLERYADDSIGNWIDTH / 2);
      y1 = mRECT.T + GALLERYADDCIRCLERADIUS - (GALLERYADDSIGNSIZE / 2);
      g.FillRect(mAddSignColor, IRECT(
        x1, y1, x1 + GALLERYADDSIGNWIDTH, y1 + GALLERYADDSIGNSIZE
      ));
      GraphStats* stats;
      MessageBus::fireEvent<GraphStats**>(MessageBus::GetGraphStats, &stats);
      avgExecutiontime = (59 * avgExecutiontime + stats->executionTime) / 60.0;
      string time = to_string(avgExecutiontime);
      g.DrawText(mStats, time.c_str(), mRECT);
      WDBGMSG("TEST %ld\n", avgExecutiontime);
      mDirty = true;
    }
  }

  void OnResize() override {
    if (mGraphics != nullptr) {
      IRECT bounds = mGraphics->GetBounds();
      if (mIsOpen) {
        bounds.Pad(-GALLERYPADDING);
        bounds.L = bounds.R * 0.5f;
        mRECT = bounds;
        mTargetRECT = bounds;
        float top = mViewPort.T;
        mViewPort = bounds;
        mViewPort.Pad(-GALLERYPADDING);
        mViewPort.T = top;
      }
      else {
        bounds.Pad(-GALLERYPADDING);
        bounds.L = bounds.R - GALLERYADDCIRCLEDIAMETER;
        bounds.B = bounds.T + GALLERYADDCIRCLEDIAMETER;
        mRECT = bounds;
        mTargetRECT = bounds;
        mViewPort = bounds;
      }
    }
  }

  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override {
    if (!mIsOpen) { return; }
    scroll(d * 20);
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    scroll(dY);
    if (dY < 2 && dX < 2) {
      mDragging = true;
    }
  }

  void OnMouseUp(float x, float y, const IMouseMod& mod) {
    if (mIsOpen && !mDragging) {
      GalleryCategory* cat;
      for (int i = 0; i < mCategories.GetSize(); i++) {
        cat = mCategories.Get(i);
        if (cat->mRECT.Contains(IRECT(x, y, x, y))) {
          NodeList::NodeInfo* ret = cat->OnMouseDown(x, y, mod);
          if (ret != nullptr) {
            MessageBus::fireEvent<NodeList::NodeInfo>(MessageBus::NodeAdd, *ret);
          }
          mDirty = true;
        }
      }
    }
    else {
      openGallery();
    }
    mDragging = false;
  }

private:
  /**
   * This will create all the categories and add the nodes to it
   */
  void init() {
    std::map<std::string, GalleryCategory*> uniqueCat;
    GalleryCategory* prevCat = nullptr;
    for (auto i : NodeList::nodelist) {
      if (uniqueCat.find(i.second.categoryName) == uniqueCat.end()) {
        GalleryCategory* cat = new GalleryCategory(prevCat, &mViewPort);
        prevCat = cat;
        uniqueCat.insert(std::pair<std::string, GalleryCategory*>(i.second.categoryName, cat));
        mCategories.Add(cat);
      }
    }
    for (auto i : NodeList::nodelist) {
      uniqueCat.at(i.second.categoryName)->addNode(i.second);
    }
  }

  void scroll(float d) {
    mViewPort.Translate(0, d);
    if (mViewPort.T > GALLERYPADDING * 2) {
      mViewPort.T = GALLERYPADDING * 2;
      //mViewPort.Translate(0, -mViewPort.T);
    }
    else {
    }
    mDirty = true;
  }

  IColor mAccentColor;
  IColor mBackgroundColor;
  IColor mAddSignColor;
  bool mIsOpen;
  bool mDragging;
  float mPadding;
  IGraphics* mGraphics;
  WDL_PtrList<GalleryCategory> mCategories;
  IRECT mViewPort;
  IRECT mViewPortBounds;
  IText mStats;
  long long avgExecutiontime;
  MessageBus::Subscription<bool> mOpenGalleryEvent;
};