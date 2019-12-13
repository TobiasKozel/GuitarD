#pragma once
#include "ReverseDelay.h"

class ReverseDelayNode final : public ReverseDelay {
public:
  ReverseDelayNode(const std::string pType) {
    shared.type = pType;
    shared.width = 300;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::SPATIAL);
  }
};
