#pragma once
#include "SimpleReverb.h"

class SimpleReverbNode : public SimpleReverb {
public:
  SimpleReverbNode(std::string pType) {
    type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(CATEGORYCOLORSPATIAL);
  }
};
