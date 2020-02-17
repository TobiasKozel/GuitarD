#pragma once
#ifndef GUITARD_HEADLESS
#include "IControl.h"
#include "../../misc/MessageBus.h"
#include "./NodeGalleryElement.h"

namespace guitard {
  class GalleryCategory : public IControl, public ScrollViewChild {
    PointerList<GalleryElement> mElements;
    bool mOpen = false;
    String mName;
    MessageBus::Bus* mBus = nullptr;
    IRECT mTitleRect;
    float mColumns = 0;
    GalleryElement* mMouseDownEl = nullptr;
  public:

    GalleryCategory(MessageBus::Bus* bus) : IControl({}) {
      mBus = bus;
    }

    ~GalleryCategory() {
      mElements.clear(true);
    }

    void addNode(NodeList::NodeInfo* node) {
      if (mName.size() == 0) {
        // Take the name of the first node, they'll all be the same
        mName = node->categoryName.c_str();
      }
      mElements.add(new GalleryElement(node));
    }

    void OnResize() override {
      // Title is always visible
      mRECT.B = mRECT.T + Theme::Gallery::ELEMENT_TITLE_HEIGHT;
      if (mOpen) {
        // Calculate own height
        mTitleRect = mRECT;
        mColumns = std::max(static_cast<int>(
          floor(mRECT.W() / (Theme::Gallery::ELEMENT_WIDTH
            + Theme::Gallery::ELEMENT_PADDING * 1.5))), 1
        );
        int rows = ceilf(mElements.size() / mColumns);
        mRECT.B +=
          rows * Theme::Gallery::ELEMENT_HEIGHT + rows
          * Theme::Gallery::ELEMENT_PADDING + Theme::Gallery::ELEMENT_PADDING;
      }
      else {
        mTitleRect = mRECT;
      }
      mTargetRECT = mRECT;
    }

    void Draw(IGraphics& g) override {
      if (mOpen) {
        if (mMouseIsOver) {
          g.FillRect(Theme::Gallery::CATEGORY_BG_HOVER, mRECT);
        }
        else {
          g.FillRect(Theme::Gallery::CATEGORY_BG, mRECT);
        }
        for (int i = 0; i < mElements.size(); i++) {
          mElements[i]->Draw(g, &mRECT, i, mColumns);
        }
      }

      if (mOpen) {
        g.FillRect(Theme::Gallery::CATEGORY_TITLE_BG_OPEN, mTitleRect);
      }
      else if (mMouseIsOver) {
        g.FillRect(Theme::Gallery::CATEGORY_TITLE_BG_HOVER, mTitleRect);
      }
      else {
        g.FillRect(Theme::Gallery::CATEGORY_TITLE_BG, mTitleRect);
      }
      g.DrawText(Theme::Gallery::CATEGORY_TITLE, mName.c_str(), mTitleRect);
    }

    void OnMouseOver(float x, float y, const IMouseMod& mod) override {
      IControl::OnMouseOver(x, y, mod);
      const IRECT click = { x, y, x, y };
      for (int i = 0; i < mElements.size(); i++) {
        GalleryElement* g = mElements[i];
        g->mMouseIsOver = g->mRECT.Contains(click);
      }
    }

    void OnMouseOut() override {
      IControl::OnMouseOut();
      mMouseDownEl = nullptr;
      mHandleDrag = false;
      for (int i = 0; i < mElements.size(); i++) {
        mElements[i]->mMouseIsOver = false;
      }
    }

    void OnMouseUp(const float x, const float y, const IMouseMod& mod) override {
      mMouseDownEl = nullptr;
      mHandleDrag = false;
      IRECT p(x, y, x, y);
      if (mTitleRect.Contains(p)) {
        mOpen = !mOpen;
        OnResize();
        return;
      }
      for (int i = 0; i < mElements.size(); i++) {
        GalleryElement* elem = mElements[i];
        if (elem->mRECT.Contains(p)) {
          MessageBus::fireEvent<NodeList::NodeInfo>(mBus, MessageBus::NodeAdd, *elem->mInfo);
        }
      }
    }

    void OnMouseDown(float x, float y, const IMouseMod& mod) override {
      IRECT p(x, y, x, y);
      for (int i = 0; i < mElements.size(); i++) {
        GalleryElement* elem = mElements[i];
        if (elem->mRECT.Contains(p)) {
          mMouseDownEl = elem;
          mHandleDrag = true;
          return;
        }
      }
      mMouseDownEl = nullptr;
    }

    void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
      if (std::abs(dX) + std::abs(dY) < 5) { return; }
      IRECT p(x, y, x, y);
      for (int i = 0; i < mElements.size(); i++) {
        GalleryElement* elem = mElements[i];
        if (elem == mMouseDownEl && elem->mRECT.Contains(p)) {
          MessageBus::fireEvent<NodeDragSpawnRequest>(mBus, MessageBus::NodeDragSpawn, {
            {x, y}, elem->mInfo->name
          });
          MessageBus::fireEvent<bool>(mBus, MessageBus::OpenGallery, false);
          return;
        }
      }
    }
  };
}
#endif