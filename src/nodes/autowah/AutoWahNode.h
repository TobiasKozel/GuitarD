#pragma once
#include "../../faust/generated/AutoWah.h"

namespace guitard {
  class AutoWahNode final : public FaustGenerated::AutoWah {
  public:
    explicit AutoWahNode(NodeList::NodeInfo* info) {
      shared.info = info;
      shared.width = 250;
      shared.height = 240;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::FILTER);
    }
#endif
  };
}