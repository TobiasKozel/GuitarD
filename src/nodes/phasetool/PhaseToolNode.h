#pragma once
#include "../../faust/generated/PhaseTool.h"

namespace guitard {
  class PhaseToolNode final : public FaustGenerated::PhaseTool {
  public:
    PhaseToolNode(const std::string pType) {
      shared.type = pType;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::TOOLS);
    }
#endif
  };
}