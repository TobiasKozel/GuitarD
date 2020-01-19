#pragma once
#include "SimpleComressor.h"

class SimpleComressorNode final : public FaustGenerated::SimpleComressor {
public:
  SimpleComressorNode(const std::string pType) {
    shared.type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::DYNAMICS);
  }
};