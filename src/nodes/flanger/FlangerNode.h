#pragma once
#include "Flanger.h"

namespace guitard {
  class FlangerNode final : public FaustGenerated::Flanger {
  public:
    FlangerNode(const std::string pType) {
      shared.type = pType;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::FILTER);
    }
#endif
  };
}
