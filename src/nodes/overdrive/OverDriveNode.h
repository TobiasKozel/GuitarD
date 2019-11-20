#pragma once
#include "OverDrive.h"

class OverDriveNode final : public OverDrive {
public:
  OverDriveNode(const std::string pType) {
    mType = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::DISTORTION);
  }
};