#pragma once
#include "../../faust/generated/Fuzz.h"

namespace guitard {
  class FuzzNode final : public FaustGenerated::Fuzz {
  public:
    FuzzNode(NodeList::NodeInfo info) {
      shared.info = info;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::DISTORTION);
    }
#endif

    std::string getLicense() override {
      std::string l = "\nFaust code from Guitarix, probably needs to be replaced/removed";
      l += Fuzz::getLicense();
      return l;
    }
  };
}