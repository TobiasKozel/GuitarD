#pragma once
#include "SimpleDrive.h"

class SimpleDriveNode : public SimpleDrive {
public:
  SimpleDriveNode(std::string pType) {
    mType = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(CATEGORYCOLORDISTORTION);
  }
};