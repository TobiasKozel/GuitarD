#pragma once
#include "../../faust/generated/PhaseTool.h"

namespace guitard {
  class PhaseToolNode final : public FaustGenerated::PhaseTool {
  public:
    PhaseToolNode(NodeList::NodeInfo info) {
      shared.info = info;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::TOOLS);
    }
#endif
  };
}