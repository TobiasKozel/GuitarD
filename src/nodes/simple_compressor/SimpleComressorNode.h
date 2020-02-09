#pragma once
#include "../../faust/generated/SimpleComressor.h"

namespace guitard {
  class SimpleComressorNode final : public FaustGenerated::SimpleComressor {
  public:
    SimpleComressorNode(const std::string pType) {
      shared.type = pType;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::DYNAMICS);
    }
#endif
  };
}