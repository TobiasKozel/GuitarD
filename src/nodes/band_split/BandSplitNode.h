#pragma once
#include "BandSplit.h"

namespace guitard {
  class BandSplitNode final : public FaustGenerated::BandSplit {
  public:
    BandSplitNode(const std::string pType) {
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
