#pragma once
#include "NodeGalleryElement.h"

using namespace iplug;
using namespace igraphics;


class GalleryCategory {
  WDL_PtrList<GalleryElement> mElements;
public:
  bool mOpen;
  const char* mName;
  // keep this one around so the c_str() of it stays valid
  std::string mNameString;
  IRECT* mViewport;
  IRECT mRECT;
  IRECT mTitleRect;
  GalleryCategory* mPrev;

  GalleryCategory(GalleryCategory* prev, IRECT* viewport) {
    mPrev = prev;
    mViewport = viewport;
    mOpen = false;
    mRECT = IRECT(0, 0, 400, Theme::Gallery::ELEMENT_TITLE_HEIGHT);
    mTitleRect = mRECT;
  }

  ~GalleryCategory() {
    mElements.Empty(true);
  }

  void OnResize() {
  }

  void addNode(const NodeList::NodeInfo node) {
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
      mRECT.T = mPrev->mRECT.B + Theme::Gallery::CATEGORY_PADDING;
    }
    else {
      mRECT.T = mViewport->T;
    }
    // Title is always visible
    mRECT.B = mRECT.T + Theme::Gallery::ELEMENT_TITLE_HEIGHT;
    if (mOpen) {
      // Calculate own height
      mTitleRect = mRECT;
      const float columns = max(static_cast<int>(
        floor(mRECT.W() / (Theme::Gallery::ELEMENT_WIDTH
        + Theme::Gallery::ELEMENT_PADDING * 1.5))), 1
      );
      int rows = ceilf(mElements.GetSize() / columns);
      mRECT.B +=
        rows * Theme::Gallery::ELEMENT_HEIGHT + rows
        * Theme::Gallery::ELEMENT_PADDING + Theme::Gallery::ELEMENT_PADDING;
      g.FillRect(Theme::Gallery::CATEGORY_BG, mRECT);
      for (int i = 0; i < mElements.GetSize(); i++) {
        mElements.Get(i)->Draw(g, &mRECT, i, columns);
      }
    }
    else {
      mTitleRect = mRECT;
    }
    g.FillRect(Theme::Gallery::CATEGORY_TITLE_BG, mTitleRect);
    g.DrawText(Theme::Gallery::CATEGORY_TITLE, mName, mTitleRect);
  }

  NodeList::NodeInfo* OnMouseDown(const float x, const float y, const IMouseMod& mod) {
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


};