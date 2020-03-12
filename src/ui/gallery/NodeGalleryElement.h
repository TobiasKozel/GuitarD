#pragma once
#ifndef GUITARD_HEADLESS
#include "../../misc/NodeList.h"
#include "../../ui/theme.h"

namespace guitard {
  struct GalleryElement {
    NodeList::NodeInfo* mInfo;
    IRECT mRECT;
    const char* mName;
    bool mMouseIsOver = false;
    bool mHasSvg = false;
    ISVG mSvgBg = ISVG(nullptr);
    float mAspectRatio = 1.0;

    GalleryElement(NodeList::NodeInfo* node, IGraphics& g) {
      mInfo = node;
      mName = mInfo->displayName.c_str();
      if (!node->image.empty()) {
        mSvgBg = g.LoadSVG(node->image.c_str());
        if (mSvgBg.IsValid()) {
          mHasSvg = true;
        }
      }
    }

    void Draw(IGraphics& g, IRECT* rect, int index, int columns) {
      //g.DrawCircle(IColor(255, 0, 100, 255), 0, 0, 10);
      mRECT = *rect;
      int row = static_cast<int>(floorf(index / static_cast<float>(columns)));
      mRECT.T +=
        (row * Theme::Gallery::ELEMENT_HEIGHT) + Theme::Gallery::ELEMENT_PADDING
        + Theme::Gallery::ELEMENT_TITLE_HEIGHT + Theme::Gallery::ELEMENT_PADDING * row;
      mRECT.B = mRECT.T + Theme::Gallery::ELEMENT_HEIGHT;
      mRECT.L +=
        (index % columns) * Theme::Gallery::ELEMENT_WIDTH +
        (index % columns + 1) * Theme::Gallery::ELEMENT_PADDING;
      mRECT.R = mRECT.L + Theme::Gallery::ELEMENT_WIDTH;
      if (mMouseIsOver) {
        g.DrawRect(iplug::igraphics::COLOR_ORANGE, mRECT);
      }
      else {
        g.DrawRect(iplug::igraphics::COLOR_WHITE, mRECT);
      }

      const IRECT padded = mRECT.GetPadded(-2);
      if (mHasSvg) {
        g.DrawSVG(mSvgBg, padded);
      }
      else {
        g.FillRoundRect(Theme::Gallery::CATEGORY_TITLE_BG_HOVER, padded, 8);
      }
      const IRECT text = padded.GetFromBottom(20);
      g.FillRect(Theme::Gallery::ELEMENT_TITLE_BG, text);
      g.DrawText(Theme::Gallery::ELEMENT_TITLE, mName, text);
    }
  };
}
#endif