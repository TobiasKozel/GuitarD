#pragma once
#include "Split.h"

class SplitNode final : public Split {
public:
  SplitNode(const std::string pType) {
    shared.type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::TOOLS);
  }
};