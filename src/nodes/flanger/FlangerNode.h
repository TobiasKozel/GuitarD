#pragma once
#include "Flanger.h"

class FlangerNode final : public Flanger {
public:
  FlangerNode(const std::string pType) {
    shared.type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::FILTER);
  }
};