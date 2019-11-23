#pragma once
#include "SimpleDrive.h"

class SimpleDriveNodeUi final : public NodeUi {
public:
  SimpleDriveNodeUi(NodeShared* param) : NodeUi(param) {
  }

  void Draw(IGraphics & g) override {
    const float val = *(shared->meters.Get(0)->value);
    const int bright = std::min(255.f, val * 1000);
    NodeUi::Draw(g);
    g.FillRect(
      IColor(255, bright, 50, 50),
      mRECT.GetPadded(-100)
    );
    mDirty = true;
  }
 
};

class SimpleDriveNode final : public SimpleDrive {
public:
  SimpleDriveNode(const std::string pType) {
    mType = pType;
  }


  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    shared.graphics = pGrahics;
    mUi = new SimpleDriveNodeUi(&shared);
    mUi->setColor(Theme::Categories::DISTORTION);
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    mUiReady = true;
  }
};