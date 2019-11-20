#pragma once
#include "SimpleDelay.h"

class SimpleDelayNode : public SimpleDelay {
public:
  SimpleDelayNode(std::string pType) {
    mType = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(CATEGORYCOLORSPATIAL);
  }
};
