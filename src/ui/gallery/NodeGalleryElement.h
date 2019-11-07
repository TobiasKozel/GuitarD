#pragma once
#include "IControl.h"
#include "src/misc/NodeList.h"
#include "src/ui/theme.h"


using namespace iplug;
using namespace igraphics;

class GalleryElement {
public:
  GalleryElement(NodeList::NodeInfo node) {
    mInfo = node;
    mName = mInfo.dislayName.c_str();
    mTitle = GALLERYELEMENTTITLE;
  }

  void Draw(IGraphics& g, IRECT* rect, int index, int columns) {
    //g.DrawCircle(IColor(255, 0, 100, 255), 0, 0, 10);
    mRECT = *rect;
    int row = static_cast<int>(floorf(index / (float)columns));
    mRECT.T += (row * GALLERYELEMENTHEIGHT) + GALLERYELEMENTPADDING + GALLERYELEMENTTITLEHEIGHT + GALLERYELEMENTPADDING * row;
    mRECT.B = mRECT.T + GALLERYELEMENTHEIGHT;
    mRECT.L += (index % columns) * GALLERYELEMENTWIDTH + (index % columns + 1) * GALLERYELEMENTPADDING;
    mRECT.R = mRECT.L + GALLERYELEMENTWIDTH;
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