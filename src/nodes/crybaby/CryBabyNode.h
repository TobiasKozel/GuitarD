#pragma once
#include "CryBaby.h"

class CryBabyNode final : public CryBaby {
public:
  CryBabyNode(const std::string pType) {
    shared.type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::FILTER);
  }
};