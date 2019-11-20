#pragma once
#include "SimpleReverb.h"

class SimpleReverbNode final : public SimpleReverb {
public:
  SimpleReverbNode(const std::string pType) {
    mType = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::SPATIAL);
  }
};
