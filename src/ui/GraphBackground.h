#pragma once
#ifndef GUITARD_HEADLESS
#include "IControl.h"
#include "../ui/theme.h"
#include "../misc/MessageBus.h"

typedef std::function<void(float x, float y, float scale)> BackgroundMoveCallback;

namespace guitard {
  class GraphBackground : public IControl {
    MessageBus::Bus* mBus;
    Coord2D mOffset;
    Coord2D mMouseDown;
    float lastWidth;
    float lastHeight;
    bool triggeredScale;
    IColor mColorBackgroundDetail;
    BackgroundMoveCallback mCallback;
  public:
    float mScale = 1;
    GraphBackground(MessageBus::Bus* pBus, BackgroundMoveCallback pCallback) :
      IControl(IRECT(), kNoParameter)
    {
      mBus = pBus;
      mCallback = pCallback;
      triggeredScale = false;
    }

    /**
     * Draw the simple background shapes
     */
    void Draw(IGraphics& g) override {
      const int windowX = g.Width();
      const int windowY = g.Height();
      g.FillRect(Theme::Graph::BACKGROUND, mRECT);
      const float dist = Theme::Graph::BACKGROUND_DETAIL_DIST;
      const float xStart = fmod(mOffset.x, dist) - dist;
      const float yStart = fmod(mOffset.y, dist) - dist;
      const float detailSize = Theme::Graph::BACKGROUND_DETAIL_SIZE;
      const float detailWidth = Theme::Graph::BACKGROUND_DETAIL_WIDTH;

      for (float y = yStart; y < windowY + dist; y += dist) {
        for (float x = xStart; x < windowX + dist; x += dist) {
          float x1 = x - (detailSize / 2.f);
          float y1 = y - (detailWidth / 2.f);
          g.FillRect(Theme::Graph::BACKGROUND_DETAIL, IRECT(
            x1, y1, x1 + detailSize, y1 + detailWidth
          ));
          x1 = x - (detailWidth / 2.f);
          y1 = y - (detailSize / 2.f);
          g.FillRect(Theme::Graph::BACKGROUND_DETAIL, IRECT(
            x1, y1, x1 + detailWidth, y1 + detailSize
          ));
        }
      }
    }

    void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
      if (mod.L) {
        if (mod.C) {
          // Allow severing connections like in blender
          MessageBus::fireEvent<Coord2D>(mBus, MessageBus::SeverNodeConnection, Coord2D{ x, y });
          return;
        }
        translate(dX, dY);
        GetUI()->SetAllControlsDirty();
      }
    }

    void OnMouseDown(float x, float y, const IMouseMod& mod) override {
      if (mod.R || mod.L) {
        MessageBus::fireEvent<bool>(mBus, MessageBus::OpenGallery, mod.R);
      }
      mMouseDown.x = x;
      mMouseDown.y = y;
    }

    void OnMouseUp(float x, float y, const IMouseMod& mod) override {
      if (std::abs(x - mMouseDown.x) + std::abs(y - mMouseDown.y) < 3) { // If we moved less than a few pixels, deselect
        MessageBus::fireEvent<NodeSelectionChanged>(
          mBus, MessageBus::NodeSelectionChange, { nullptr, true }
        );
      }
    }


    void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override {
      /**
       * TODOG sucks hard, at least some kind of scaling
       * We're essentially using the renderer to scale the graph (and everything else)
       * This also changes the window size, so here's some counter scaling
       * to keep it roughly the same. It does some resizeing because of rounding errors though.
       */
      float newScale = mScale + d / 20.f;
      float w = lastWidth;
      float h = lastHeight;
      if (newScale > 0.45 && newScale < 2) {
        if (w / newScale >= PLUG_MAX_WIDTH && h / newScale >= PLUG_MAX_HEIGHT) {
          return;
        }
        if (w / newScale <= PLUG_MIN_WIDTH && h / newScale <= PLUG_MIN_HEIGHT) {
          return;
        }

        lastWidth = ((w * mScale) / newScale);
        lastHeight = ((h * mScale) / newScale);

        float dX = ((x * mScale) / newScale) - x;
        float dY = ((y * mScale) / newScale) - y;


        triggeredScale = true;

        GetUI()->Resize(ceil(lastWidth), ceil(lastHeight), newScale, true);
        translate(dX, dY);
        mScale = newScale;
      }
    }

    void OnResize() override {
      if (true) {
        if (!triggeredScale) {
          storeSize();
        }
        triggeredScale = false;
        IRECT bounds = GetUI()->GetBounds();
        mRECT = bounds;
        mTargetRECT = bounds;
      }
    }

  protected:
    void translate(float dX, float dY) {
      mOffset.x += dX;
      mOffset.y += dY;
      mCallback(dX, dY, 1.f);
    }

    void storeSize() {
      lastHeight = GetUI()->Height();
      lastWidth = GetUI()->Width();
    }
  };
}
#endif