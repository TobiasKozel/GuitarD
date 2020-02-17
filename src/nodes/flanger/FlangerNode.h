#pragma once
#include "../../faust/generated/Flanger.h"

namespace guitard {
  class FlangerNode final : public FaustGenerated::Flanger {
  public:
    FlangerNode(NodeList::NodeInfo info) {
      shared.info = info;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::FILTER);
    }
#endif
  };
}
