#pragma once
#include "../../faust/generated/ReverseDelay.h"

namespace guitard {
  class ReverseDelayNode final : public FaustGenerated::ReverseDelay {
  public:
    ReverseDelayNode(NodeList::NodeInfo info) {
      shared.info = info;
      shared.width = 300;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::SPATIAL);
    }
#endif

    std::string getLicense() override {
      std::string l = "Code from https://gist.github.com/tomoyanonymous\n";
      l += "Gist https://gist.github.com/tomoyanonymous/d527fca58e929de6a021565505589406\n";
      l += "No license provided, need to ask for permission";
      l += ReverseDelay::getLicense();
      return l;
    }
  };
}