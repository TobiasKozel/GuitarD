#pragma once
#include "PhaseTool.h"

class PhaseToolNode final : public FaustGenerated::PhaseTool {
 public:
  PhaseToolNode(const std::string pType) {
    shared.type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(Theme::Categories::TOOLS);
  }
};