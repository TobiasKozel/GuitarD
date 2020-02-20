#pragma once
#include "../../faust/generated/Transpose.h"

namespace guitard {
  class TransposeNode final : public FaustGenerated::Transpose {
  public:
    TransposeNode(NodeList::NodeInfo* info) {
      shared.info = info;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::FILTER);
    }
#endif
  };
}