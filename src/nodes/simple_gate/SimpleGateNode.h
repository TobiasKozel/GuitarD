#pragma once
#include "../../faust/generated/SimpleGate.h"

namespace guitard {
  class SimpleGateNode final : public FaustGenerated::SimpleGate {
  public:
    SimpleGateNode(NodeList::NodeInfo info) {
      shared.info = info;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::DYNAMICS);
    }
#endif
  };
}