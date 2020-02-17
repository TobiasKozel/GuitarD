#pragma once
#include "../../faust/generated/OverDrive.h"
namespace guitard {
  class OverDriveNode final : public FaustGenerated::OverDrive {
  public:
    OverDriveNode(NodeList::NodeInfo* info) {
      shared.info = info;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::DISTORTION);
    }
#endif
  };
}