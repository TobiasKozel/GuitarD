#pragma once
#include "PhaseTool.h"

class PhaseToolNode : public PhaseTool {
 public:
  PhaseToolNode(std::string pType) {
    mType = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(CATEGORYCOLORTOOLS);
  }
};