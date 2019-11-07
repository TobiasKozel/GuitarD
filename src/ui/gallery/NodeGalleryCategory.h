#pragma once
#include "NodeGalleryElement.h"

using namespace iplug;
using namespace igraphics;


class GalleryCategory {
  WDL_PtrList<GalleryElement> mElements;
public:
  GalleryCategory(GalleryCategory* prev, IRECT* viewport) {
    mTitleBack = IColor(255, GALLERYCATEGORYTITLEBACKROUND);
    mBack = IColor(255, GALLERYCATEGORYBACKGROUND);
    mPrev = prev;
    mViewport = viewport;
    mOpen = false;
    mRECT = IRECT(0, 0, 400, GALLERYELEMENTTITLEHEIGHT);
    mTitleRect = mRECT;
    mTitle = GALLERYCATEGORYTITLE;
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
    // See at which y the category starts
    if (mPrev != nullptr) {
      mRECT.T = mPrev->mRECT.B + GALLERYCATEGORYPADDING;
    }
    else {
      mRECT.T = mViewport->T;
    }
    // Title is always visible
    mRECT.B = mRECT.T + GALLERYELEMENTTITLEHEIGHT;
    if (mOpen) {
      // Calculate own height
      mTitleRect = mRECT;
      float columns = max(static_cast<int>(floor(mRECT.W() / GALLERYELEMENTWIDTH)), 1);
      int rows = ceilf(mElements.GetSize() / columns);
      mRECT.B += rows * GALLERYELEMENTHEIGHT + rows * GALLERYELEMENTPADDING + GALLERYELEMENTPADDING;
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
    return nullptr;
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