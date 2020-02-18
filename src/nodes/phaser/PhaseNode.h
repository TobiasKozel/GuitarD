#pragma once
#include "../../faust/generated/Phaser.h"

namespace guitard {
  class PhaserNode final : public FaustGenerated::Phaser {
  public:
    explicit PhaserNode(NodeList::NodeInfo* info) {
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