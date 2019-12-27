#pragma once
#include "IControl.h"
#include "src/misc/NodeList.h"
#include "src/ui/theme.h"


using namespace iplug;
using namespace igraphics;

struct GalleryElement {
  NodeList::NodeInfo mInfo;
  IRECT mRECT;
  IBitmap* mBitmap;
  const char* mName;
  const char* mImage;
  bool mMouseIsOver = false;

  GalleryElement(NodeList::NodeInfo node) {
    mInfo = node;
    mName = mInfo.displayName.c_str();
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
      g.DrawRect(COLOR_ORANGE, mRECT);
    }
    else {
      g.DrawRect(COLOR_WHITE, mRECT);
    }
    g.DrawText(Theme::Gallery::ELEMENT_TITLE, mName, mRECT);
  }
};