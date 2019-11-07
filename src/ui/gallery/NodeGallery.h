#pragma once
#include "IControl.h"
#include "src/node/Node.h"
#include "src/misc/MessageBus.h"
#include "NodeGalleryCategory.h"
#include "src/ui/theme.h"

using namespace iplug;
using namespace igraphics;


class NodeGallery : public IControl {
public:
  NodeGallery(IGraphics* g) :
    IControl(IRECT(), kNoParameter)
  {
    mAccentColor = IColor(255, COLORACCENT);
    mBackgroundColor = IColor(255, GALLERYBACKGROUND);
    mPadding = 10;
    mGraphics = g;
    init();
    mIsOpen = false;
    OnResize();
    mOpenGalleryEvent.subscribe("OpenGallery", [&](bool open) {
      this->openGallery(open);
    });
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
    }
    else {
      g.FillCircle(mAccentColor, mRECT);
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
        mViewPort = bounds;
        mViewPort.Pad(-GALLERYPADDING);
      }
      else {
        bounds.Pad(-GALLERYPADDING);
        bounds.L = bounds.R - 60;
        bounds.B = bounds.T + 60;
        mRECT = bounds;
        mTargetRECT = bounds;
        mViewPort = bounds;
      }
    }
  }

  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override {
    if (!mIsOpen) { return; }
    float scroll = d * 20;
    if (mod.C) {
      mViewPort.Translate(scroll, 0);
    }
    else {
      mViewPort.Translate(0, scroll);
      if (mViewPort.T > 0) {
        mViewPort.Translate(0, -mViewPort.T);
      }
      else {
      }
    }
    mDirty = true;
  }

  void OnMouseUp(float x, float y, const IMouseMod& mod) {
    if (mIsOpen) {
      GalleryCategory* cat;
      for (int i = 0; i < mCategories.GetSize(); i++) {
        cat = mCategories.Get(i);
        if (cat->mRECT.Contains(IRECT(x, y, x, y))) {
          NodeList::NodeInfo* ret = cat->OnMouseDown(x, y, mod);
          if (ret != nullptr) {
            MessageBus::fireEvent<NodeList::NodeInfo>("NodeAdd", *ret);
          }
          mDirty = true;
        }
      }
    }
    else {
      openGallery();
    }
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
  IColor mAccentColor;
  IColor mBackgroundColor;
  bool mIsOpen;
  float mPadding;
  IGraphics* mGraphics;
  WDL_PtrList<GalleryCategory> mCategories;
  IRECT mViewPort;
  IRECT mViewPortBounds;
  MessageBus::Subscription<bool> mOpenGalleryEvent;
};