#pragma once
#include "CryBaby.h"

class CryBabyNode : public CryBaby {
public:
  CryBabyNode(std::string pType) {
    mType = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(CATEGORYCOLORFILTER);
  }
};