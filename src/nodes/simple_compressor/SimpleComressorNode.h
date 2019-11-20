#pragma once
#include "SimpleComressor.h"

class SimpleComressorNode final : public SimpleComressor {
public:
  SimpleComressorNode(const std::string pType) {
    mType = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::DYNAMICS);
  }
};