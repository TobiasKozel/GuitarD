#pragma once
#include "Fuzz.h"

class FuzzNode final : public Fuzz {
public:
  FuzzNode(const std::string pType) {
    shared.type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::DISTORTION);
  }
};