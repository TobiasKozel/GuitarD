#pragma once
#include "../../faust/generated/Split.h"

namespace guitard {
  class SplitNode final : public FaustGenerated::Split {
  public:
    SplitNode(NodeList::NodeInfo info) {
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
