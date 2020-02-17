#pragma once
#include "../../faust/generated/CryBaby.h"

namespace guitard {
  class CryBabyNode final : public FaustGenerated::CryBaby {
  public:
    CryBabyNode(NodeList::NodeInfo* info) {
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
