#pragma once
#include "../../faust/generated/Split.h"

namespace guitard {
  class SplitNode final : public FaustGenerated::Split {
  public:
    SplitNode(const std::string pType) {
      shared.type = pType;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::TOOLS);
    }
#endif
  };
}
