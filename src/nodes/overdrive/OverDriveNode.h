#pragma once
#include "OverDrive.h"

class OverDriveNode : public OverDrive {
public:
  OverDriveNode(std::string pType) {
    type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(CATEGORYCOLORDISTORTION);
  }
};