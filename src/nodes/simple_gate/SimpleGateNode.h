#pragma once
#include "SimpleGate.h"

namespace guitard {
  class SimpleGateNode final : public FaustGenerated::SimpleGate {
  public:
    SimpleGateNode(const std::string pType) {
      shared.type = pType;
    }

    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::DYNAMICS);
    }
  };
}