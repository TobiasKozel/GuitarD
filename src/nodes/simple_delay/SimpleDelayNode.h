#pragma once
#include "../../faust/generated/SimpleDelay.h"

namespace guitard {
  class SimpleDelayNode final : public FaustGenerated::SimpleDelay {
  public:
    SimpleDelayNode(NodeList::NodeInfo* info) {
      shared.info = info;
      shared.width = 300;
    }

//#ifndef GUITARD_HEADLESS
//    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
//      Node::setupUi(pGrahics);
//      mUi->setColor(Theme::Categories::SPATIAL);
//    }
//#endif
  };
}

GUITARD_REGISTER_NODE(SimpleDelayNode,
  "Basic Delay", "Delays/Reverbs", "A very simple delay effect", "image"
)