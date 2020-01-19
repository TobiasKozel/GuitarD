#pragma once
#include "BandSplit.h"

class BandSplitNode final : public FaustGenerated::BandSplit {
public:
  BandSplitNode(const std::string pType) {
    shared.type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::TOOLS);
  }
};