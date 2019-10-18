#pragma once
#include "IControl.h"
#include "src/graph/Node.h"
#include "src/graph/nodes/NodeList.h"
#include "config.h"

typedef std::function<void(const char* nodeName)> GalleryAddCallBack;

class GalleryElement {
public:
  GalleryElement(float x, float y, const char* pName, IBitmap* pBitmap) {
    mRect.L = x;
    mRect.T = y;
    mRect.R = x + pBitmap->W();
    mRect.B = y + pBitmap->H();
    mBitmap = pBitmap;
    mName = pName;
  }
  IRECT mRect;
  IBitmap* mBitmap;
  const char* mName;
  const char* mImage;
};

using namespace iplug;
using namespace igraphics;


class NodeGallery : public IControl {
public:
  NodeGallery(IGraphics* g, IRECT size, GalleryAddCallBack callback) :
    IControl(size, kNoParameter)
  {
    mPadding = 10;
    mGraphics = g;
    mCallback = callback;
    mViewPort = size;
    addBelow();
    addBelow();
    addBelow();
  }


  virtual void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    mCallback("asd");
  }

  void Draw(IGraphics& g) override {
    g.DrawRect(IColor(255, 255, 0, 255), mRECT);

    for (int i = 0; i < mElements.GetSize(); i++) {
      IRECT cur = mElements.Get(i)->mRect;
      cur.Translate(mViewPort.L, mViewPort.T);
      if (mRECT.Contains(cur) || true) {
        g.FillRect(IColor(255, 0, 0, 255), cur);
      }
    }
  }

  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override {
    mViewPort.T += d * 2;

    mDirty = true;
  }

  void addBelow() {
    float yOff = 0;
    for (int i = 0; i < mElements.GetSize(); i++) {
      float b = mElements.Get(i)->mRect.B;
      yOff = yOff < b ? b : yOff;
    }
    auto mBitmap = mGraphics->LoadBitmap(PNGGENERICBG_FN, 1, false);
    GalleryElement* elem = new GalleryElement(0, yOff + mPadding, "asd", &mBitmap);
    float b = elem->mRect.B;
    mViewPort.B = b > mViewPort.B ? b : mViewPort.B;
    float r = elem->mRect.B;
    mViewPort.R = r > mViewPort.R ? r : mViewPort.R;
    mElements.Add(elem);
  }

private:
  float mPadding;
  GalleryAddCallBack mCallback;
  IGraphics* mGraphics;
  WDL_PtrList<GalleryElement> mElements;
  IRECT mViewPort;
};