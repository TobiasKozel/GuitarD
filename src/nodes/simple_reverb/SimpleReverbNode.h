#pragma once
#include "SimpleReverb.h"

class SimpleReverbNode final : public FaustGenerated::SimpleReverb {
public:
  SimpleReverbNode(const std::string pType) {
    shared.type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::SPATIAL);
  }
};
