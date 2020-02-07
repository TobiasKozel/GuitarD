#pragma once
#include "IControl.h"
#include "src/ui/theme.h"
#include "src/misc/MessageBus.h"

typedef std::function<void(float x, float y, float scale)> BackgroundMoveCallback;

namespace guitard {
  class GraphBackground : public IControl {
    MessageBus::Bus* mBus;
    IGraphics* mGraphics;
    float mX = 0;
    float mY = 0;
    float offsetX = 0;
    float offsetY = 0;
    float lastWidth;
    float lastHeight;
    bool triggeredScale;
    IColor mColorBackgroundDetail;
    BackgroundMoveCallback mCallback;
  public:
    float mScale = 1;
    GraphBackground(MessageBus::Bus* pBus, IGraphics* g, BackgroundMoveCallback pCallback) :
      IControl(IRECT(0, 0, g->Width(), g->Height()), kNoParameter)
    {
      mBus = pBus;
      mGraphics = g;
      mCallback = pCallback;
      storeSize();
      triggeredScale = false;
    }

    /**
     * Draw the simple background shapes
     */
    void Draw(IGraphics& g) override {
      int windowX = g.Width();
      int windowY = g.Height();
      g.FillRect(Theme::Graph::BACKGROUND, mRECT);
      const float dist = Theme::Graph::BACKGROUND_DETAIL_DIST;
      const float yStart = fmod(offsetY, dist) - dist;
      const float xStart = fmod(offsetX, dist) - dist;
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
        mGraphics->SetAllControlsDirty();
      }
    }

    void OnMouseDown(float x, float y, const IMouseMod& mod) override {
      if (mod.R || mod.L) {
        MessageBus::fireEvent<bool>(mBus, MessageBus::OpenGallery, mod.R);
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

        mGraphics->Resize(ceil(lastWidth), ceil(lastHeight), newScale, true);
        translate(dX, dY);
        mScale = newScale;
      }
    }

    void OnResize() override {
      if (mGraphics != nullptr) {
        if (!triggeredScale) {
          storeSize();
        }
        triggeredScale = false;
        IRECT bounds = mGraphics->GetBounds();
        mRECT = bounds;
        mTargetRECT = bounds;
      }
    }

  protected:
    void translate(float dX, float dY) {
      offsetX += (dX);
      offsetY += (dY);
      mCallback(dX, dY, 1.f);
    }

    void storeSize() {
      lastHeight = mGraphics->Height();
      lastWidth = mGraphics->Width();
    }
  };
}