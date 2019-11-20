#pragma once
#include "config.h"
#include "IPlugStructs.h"

using namespace iplug;
using namespace igraphics;

namespace Theme {
  namespace Colors {
    const IColor ACCENT(255, 233, 140, 36);
  }
  namespace Graph {
    const IColor BACKGROUND(255, 230, 230, 230);
    const IColor BACKGROUND_DETAIL(255, 200, 200, 200);
    const float BACKGROUND_DETAIL_DIST = 140.f;
    const float BACKGROUND_DETAIL_SIZE = 12;
    const float BACKGROUND_DETAIL_WIDTH = 1.5;
  }

  namespace Gallery {
    const float GALLERY_ADD_CIRCLE_DIAMETER = 48.f;
    const float GALLERY_ADD_CIRCLE_RADIUS = GALLERY_ADD_CIRCLE_DIAMETER * 0.5f;
    const IColor COLOR_GALLERY_BACKGROUND(255, 180, 180, 180);
    const float BUTTON_SIZE = 18;
    const float BUTTON_ICON_SIZE = 3;
    const float PADDING = 10;

    const IText ELEMENT_TITLE{ 18, COLOR_WHITE, "Roboto-Regular", EAlign::Center, EVAlign::Bottom, 0 };
    const float ELEMENT_TITLE_HEIGHT = 28;
    const float ELEMENT_HEIGHT = 110;
    const float ELEMENT_WIDTH = 200;
    const float ELEMENT_PADDING = 8;

    const IColor CATEGORY_TITLE_BG(255, 100, 100, 100);
    const IText CATEGORY_TITLE{ 24, COLOR_WHITE, "Roboto-Regular", EAlign::Center, EVAlign::Middle, 0 };
    const float CATEGORY_PADDING = 8;
    const IColor CATEGORY_BG(255, 30, 30, 30);
  }

  namespace Categories {
    const IColor TOOLS(255, 100, 100, 220);
    const IColor FILTER(255, 50, 150, 70);
    const IColor DISTORTION(255, 200, 80, 60);
    const IColor DYNAMICS(255, 120, 50, 130);
    const IColor SPATIAL(255, 60, 50, 160);
  }

  namespace Cables {
    const float THICKNESS = 5;
    const float PREVIEW_DASH_DIST = 20;
    const IColor COLOR(255, 40, 40, 40);
    const IColor COLOR_SPLICE_IN(255, 250, 150, 0);
    const IColor COLOR_PREVIEW(150, 40, 40, 40);
  }

  namespace Sockets {
    const float DIAMETER = 30.f;
    const float RADIUS = DIAMETER * 0.5f;
    const float ACTIVE_SIZE = RADIUS * 0.7f;
    const float OUTLINE_SIZE = 1.1f;
    const IColor COLOR(255, 40, 40, 40);
    const IColor COLOR_INNER(255, 250, 250, 250);
    const IColor COLOR_ACTIVE(255, 250, 150, 0);
  }

  namespace  Node {
#define NODE_CACHE_BG
#define NODE_ROUNDED_CORNER
#define NODE_SHADOW
    const IColor HEADER{ 255, 30, 30, 30 };
    const float HEADER_SIZE = 20;
    const float HEADER_BYPASS_LEFT = 6;
    const float HEADER_BYPASS_TOP = 2;
    const float HEADER_BYPASS_SIZE = 16;
    const float HEADER_DISCONNECT_RIGHT = 26;
    const float HEADER_DISCONNECT_TOP = 2;
    const float HEADER_DISCONNECT_SIZE = 20;
    const float HEADER_REMOVE_RIGHT = 6;
    const float SHADOW_DIST_X = 2.f;
    const float SHADOW_DIST_Y = 2.f;
    const float SHADOW_BOUNDS = 15.f;
    const float SHADOW_BLUR = 15.f;
    const float ROUNDED_CORNER = 15.f;
    const IColor SHADOW_COLOR(255, 0, 0, 0);
  }
}

#define DEBUG_FONT IText { 16, COLOR_BLACK, "Roboto-Regular", iplug::igraphics::EAlign::Center, iplug::igraphics::EVAlign::Middle, 0 }
#define ICON_FONT IText {16.f, COLOR_WHITE, "ForkAwesome"}
