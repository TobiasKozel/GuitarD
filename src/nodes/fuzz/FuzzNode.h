#pragma once
#include "Fuzz.h"

class FuzzNode : public Fuzz {
public:
  FuzzNode(std::string pType) {
    type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(CATEGORYCOLORDISTORTION);
  }
};