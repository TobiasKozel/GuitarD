#pragma once
#include "OverDrive.h"

class OverDriveNode final : public FaustGenerated::OverDrive {
public:
  OverDriveNode(const std::string pType) {
    shared.type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::DISTORTION);
  }
};