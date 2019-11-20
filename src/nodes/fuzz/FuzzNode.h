#pragma once
#include "Fuzz.h"

class FuzzNode : public Fuzz {
public:
  FuzzNode(std::string pType) {
    mType = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(CATEGORYCOLORDISTORTION);
  }
};