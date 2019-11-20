#pragma once
#include "SimpleDrive.h"

class SimpleDriveNode final : public SimpleDrive {
public:
  SimpleDriveNode(const std::string pType) {
    mType = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::DISTORTION);
  }
};