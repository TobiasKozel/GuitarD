#pragma once
#include "PhaseTool.h"

class PhaseToolNode final : public PhaseTool {
 public:
  PhaseToolNode(const std::string pType) {
    mType = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::TOOLS);
  }
};