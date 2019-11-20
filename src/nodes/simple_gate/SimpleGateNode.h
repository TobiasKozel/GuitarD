#pragma once
#include "SimpleGate.h"

class SimpleGateNode final : public SimpleGate {
public:
  SimpleGateNode(const std::string pType) {
    mType = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::DYNAMICS);
  }
};