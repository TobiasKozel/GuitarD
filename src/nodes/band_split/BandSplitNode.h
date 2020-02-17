#pragma once
#include "../../faust/generated/BandSplit.h"

namespace guitard {
  class BandSplitNode final : public FaustGenerated::BandSplit {
  public:
    BandSplitNode(NodeList::NodeInfo info) {
      shared.info = info;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::TOOLS);
    }
#endif
  };
}
