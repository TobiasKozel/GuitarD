#pragma once
#include "IControl.h"
#include "src/graph/Node.h"
#include "src/graph/misc/NodeList.h"
#include "config.h"

using namespace iplug;
using namespace igraphics;


class GalleryElement {
public:
  GalleryElement(NodeList::NodeInfo node) {
    //mRect.L = x;
    //mRect.T = y;
    //mRect.R = x + pBitmap->W();
    //mRect.B = y + pBitmap->H();
    //mBitmap = pBitmap;
    //mName = pName;
  }

  void Draw(IGraphics& g) {
    g.DrawCircle(IColor(255, 0, 100, 255), 0, 0, 10);
  }
  IRECT mRECT;
  IBitmap* mBitmap;
  const char* mName;
  const char* mImage;
};

#define TITLEHEIGHT 40
#define CATEGORYPADDING 20
class GalleryCategory {
  WDL_PtrList<GalleryElement> mElements;
public:
  GalleryCategory(GalleryCategory* prev) {
    mPrev = prev;
    mTitleBack = IColor(255, 100, 100, 100);
    mOpen = false;
    mRECT = IRECT(0, 0, 400, TITLEHEIGHT);
    mTitleRect = mRECT;
    OnResize();
  }

  void OnResize() {
    mRECT.L = 0;
    mRECT.R = 400;
    float fromTop = mRECT.T;
    if (mPrev != nullptr) {
      fromTop -= mPrev->mRECT.B - CATEGORYPADDING;
    }
    mRECT.T = 0;
    mRECT.B -= fromTop;
  }

  void addNode(NodeList::NodeInfo node) {
    mName = node.categoryName.c_str();
    mElements.Add(new GalleryElement(node));
  }

  void Draw(IGraphics& g, IRECT &viewport, IRECT& bounds) {
    if (mOpen) {
      g.FillRect(mTitleBack, mRECT);
      return;
      GalleryElement* elem;
      for (int i = 0; i < mElements.GetSize(); i++) {
        elem = mElements.Get(i);
        if (bounds.Contains(elem->mRECT)) {
          elem->Draw(g);
        }
      }
    }
    g.FillRect(mTitleBack, mTitleRect);
    g.DrawText(mTitle, mName, mTitleRect);
  }

  void OnMouseDown(float x, float y, const IMouseMod& mod) {

  }

  IText mTitle;
  bool mOpen;
  const char* mName;
  IRECT mRECT;
  IColor mTitleBack;
  IRECT mTitleRect;
  GalleryCategory* mPrev;
};


typedef std::function<void(const char* nodeName)> GalleryAddCallBack;

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




  void Draw(IGraphics& g) override {
    g.FillRect(IColor(255, 50, 50, 50), mRECT);

    for (int i = 0; i < mCategories.GetSize(); i++) {
      mCategories.Get(i)->Draw(g, mViewPort, mRECT);
    }
  }



  void addBelow() {
    //float yOff = 0;
    //for (int i = 0; i < mElements.GetSize(); i++) {
    //  float b = mElements.Get(i)->mRect.B;
    //  yOff = yOff < b ? b : yOff;
    //}
    //auto mBitmap = mGraphics->LoadBitmap(PNGGENERICBG_FN, 1, false);
    //GalleryElement* elem = new GalleryElement(0, yOff + mPadding, "asd", &mBitmap);
    //float b = elem->mRect.B;
    //mViewPort.B = b > mViewPort.B ? b : mViewPort.B;
    //float r = elem->mRect.B;
    //mViewPort.R = r > mViewPort.R ? r : mViewPort.R;
    //mElements.Add(elem);
  }

  virtual void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    mCallback("asd");
  }

  void OnResize() override {
    if (mGraphics != nullptr) {
      IRECT bounds = mGraphics->GetBounds();
      bounds.L = bounds.R * 0.5;
      mRECT = bounds;
      mTargetRECT = bounds;
    }
  }

  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override {
    float scroll = d * 20;
    if (mod.C) {
      mViewPort.Translate(scroll, 0);
    }
    else {
      mViewPort.Translate(0, scroll);
    }
    mDirty = true;
  }

  void OnMouseDown(float x, float y, const IMouseMod& mod) {
    GalleryCategory* cat;
    for (int i = 0; i < mCategories.GetSize(); i++) {
      cat = mCategories.Get(i);
      if (cat->mRECT.Contains(IRECT(x, y, x, y))) {
        cat->OnMouseDown(x, y, mod);
      }
    }
  }

private:
  void init() {
    std::map<std::string, GalleryCategory*> uniqueCat;
    GalleryCategory* prevCat = nullptr;
    for (auto i : NodeList::nodelist) {
      if (uniqueCat.find(i.second.categoryName) == uniqueCat.end()) {
        GalleryCategory* cat = new GalleryCategory(prevCat);
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
};