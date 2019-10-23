#pragma once
#include "IControl.h"
#include "src/graph/Node.h"
#include "src/graph/misc/NodeList.h"
#include "config.h"
#include "src/logger.h"

using namespace iplug;
using namespace igraphics;


#define ELEMENTHEIGHT 110
#define ELEMENTWIDTH 200
#define ELEMENTPADDING 8
#define TITLEHEIGHT 28

class GalleryElement {
public:
  GalleryElement(NodeList::NodeInfo node) {
    mInfo = node;
    mName = mInfo.dislayName.c_str();
    mTitle = IText{
      18, COLOR_WHITE, "Roboto-Regular", EAlign::Center, EVAlign::Bottom, 0
    };
  }

  void Draw(IGraphics& g, IRECT* rect, int index, int columns) {
    //g.DrawCircle(IColor(255, 0, 100, 255), 0, 0, 10);
    mRECT = *rect;
    int row = static_cast<int>(floorf(index / (float)columns));
    mRECT.T += (row * ELEMENTHEIGHT) + ELEMENTPADDING + TITLEHEIGHT + ELEMENTPADDING * row;
    mRECT.B = mRECT.T + ELEMENTHEIGHT;
    mRECT.L += (index % columns) * ELEMENTWIDTH + (index % columns + 1) * ELEMENTPADDING;
    mRECT.R = mRECT.L + ELEMENTWIDTH;
    g.DrawRect(COLOR_WHITE, mRECT);
    g.DrawText(mTitle, mName, mRECT);
  }
  NodeList::NodeInfo mInfo;
  IRECT mRECT;
  IBitmap* mBitmap;
  const char* mName;
  const char* mImage;
  IText mTitle;
};

#define CATEGORYPADDING 8

class GalleryCategory {
  WDL_PtrList<GalleryElement> mElements;
public:
  GalleryCategory(GalleryCategory* prev, IRECT* viewport) {
    mTitleBack = IColor(255, 100, 100, 100);
    mBack = IColor(255, 30, 30, 30);
    mPrev = prev;
    mViewport = viewport;
    mOpen = false;
    mRECT = IRECT(0, 0, 400, TITLEHEIGHT);
    mTitleRect = mRECT;
    mTitle = IText {
      24, COLOR_WHITE, "Roboto-Regular", EAlign::Center, EVAlign::Middle, 0
    };
  }

  ~GalleryCategory() {
    mElements.Empty(true);
  }

  void OnResize() {
  }

  void addNode(NodeList::NodeInfo node) {
    mNameString = node.categoryName;
    mName = mNameString.c_str();
    mElements.Compact();
    mElements.Add(new GalleryElement(node));
  }

  void Draw(IGraphics& g) {
    mRECT.L = mViewport->L;
    mRECT.R = mViewport->R;
    if (mPrev != nullptr) {
      mRECT.T = mPrev->mRECT.B + CATEGORYPADDING;
    }
    else {
      mRECT.T = mViewport->T + CATEGORYPADDING;
    }
    mRECT.B = mRECT.T + TITLEHEIGHT;
    if (mOpen) {
      int columns = static_cast<int>(floorf(mRECT.W() / ELEMENTWIDTH));
      if (columns == 0) { columns = 1; }
      mTitleRect = mRECT;
      mRECT.B += ceilf(mElements.GetSize() / (float) columns) * ELEMENTHEIGHT + ELEMENTPADDING * 2;
      g.FillRect(mBack, mRECT);
      for (int i = 0; i < mElements.GetSize(); i++) {
        mElements.Get(i)->Draw(g, &mRECT, i, columns);
      }
    }
    else {
      mTitleRect = mRECT;
    }
    g.FillRect(mTitleBack, mTitleRect);
    g.DrawText(mTitle, mName, mTitleRect);
  }

  NodeList::NodeInfo* OnMouseDown(float x, float y, const IMouseMod& mod) {
    IRECT p(x, y, x, y);
    if (mTitleRect.Contains(p)) {
      mOpen = !mOpen;
      return nullptr;
    }
    for (int i = 0; i < mElements.GetSize(); i++) {
      GalleryElement* elem = mElements.Get(i);
      if (elem->mRECT.Contains(p)) {
        return &(elem->mInfo);
      }
    }
  }

  IText mTitle;
  bool mOpen;
  const char* mName;
  // keep this one around so the c_str() of it stays valid
  std::string mNameString;
  IColor mTitleBack;
  IColor mBack;
  IRECT* mViewport;
  IRECT mRECT;
  IRECT mTitleRect;
  GalleryCategory* mPrev;
};


typedef std::function<void(NodeList::NodeInfo info)> GalleryAddCallBack;

#define GALLERYPADDING 10

class NodeGallery : public IControl {
public:
  NodeGallery(IGraphics* g, GalleryAddCallBack callback) :
    IControl(IRECT(), kNoParameter)
  {
    mPadding = 10;
    mGraphics = g;
    mCallback = callback;
    init();
    OnResize();
  }

  ~NodeGallery() {
    mCategories.Empty(true);
  }

  void Draw(IGraphics& g) override {
    g.FillRect(IColor(255, 50, 50, 50), mRECT);

    for (int i = 0; i < mCategories.GetSize(); i++) {
      mCategories.Get(i)->Draw(g);
    }
  }

  void OnResize() override {
    if (mGraphics != nullptr) {
      IRECT bounds = mGraphics->GetBounds();
      bounds.Pad(-GALLERYPADDING);
      bounds.L = bounds.R * 0.5f;
      mRECT = bounds;
      mTargetRECT = bounds;
      mViewPort = bounds;
      mViewPort.Pad(-GALLERYPADDING);
    }
  }

  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override {
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
    GalleryCategory* cat;
    for (int i = 0; i < mCategories.GetSize(); i++) {
      cat = mCategories.Get(i);
      if (cat->mRECT.Contains(IRECT(x, y, x, y))) {
        NodeList::NodeInfo* ret = cat->OnMouseDown(x, y, mod);
        if (ret != nullptr) {
          mCallback(*ret);
        }
        mDirty = true;
      }
    }
  }

private:
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

  float mPadding;
  GalleryAddCallBack mCallback;
  IGraphics* mGraphics;
  WDL_PtrList<GalleryCategory> mCategories;
  IRECT mViewPort;
  IRECT mViewPortBounds;
};