#pragma once
#include "SimpleDelay.h"

namespace guitard {
  class SimpleDelayNode final : public FaustGenerated::SimpleDelay {
  public:
    SimpleDelayNode(const std::string pType) {
      shared.type = pType;
      shared.width = 300;
    }

    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::SPATIAL);
    }
  };
}