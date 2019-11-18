#pragma once
#include "SimpleComressor.h"

class SimpleComressorNode : public SimpleComressor {
public:
  SimpleComressorNode(std::string pType) {
    type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(CATEGORYCOLORDYNAMICS);
  }
};