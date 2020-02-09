#pragma once
#include "../../faust/generated/SimpleReverb.h"

namespace guitard {
  class SimpleReverbNode final : public FaustGenerated::SimpleReverb {
  public:
    SimpleReverbNode(const std::string pType) {
      shared.type = pType;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::SPATIAL);
    }
#endif
  };
}