#pragma once
#include "SimpleDrive.h"

class SimpleDriveNodeUi final : public NodeUi {
  double last = 0;
  const double speed = 0.4;
public:
  SimpleDriveNodeUi(NodeShared* param) : NodeUi(param) {
  }

  void Draw(IGraphics & g) override {
    const double val = *(shared->meters[0]->value) * 600;
    last = val * speed + (1 - speed) * last;
    const int bright = std::min<int>(255, last);
    NodeUi::Draw(g);
    const float x = mRECT.L + 125;
    const float y = mRECT.T + 80;
    const IRECT pos = { x, y, x + 30, y + 30 };
    g.FillRect(
      IColor(255, bright, 50, 50),
      IRECT( x, y, x + 30, y + 30)
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
    const float size = 60;
    const float left1 = shared.width * 0.26 - shared.width / 2.f;
    const float left2 = shared.width * 0.75 - shared.width / 2.f;
    const float center = (left1 + left2) / 2.f;
    const float top1 = -35;
    const float top2 = 40;
    mUi->mParamsByName.at("Drive")->setPos(left1, top1, size);
    mUi->mParamsByName.at("Offset")->setPos(left2, top1, size);
    mUi->mParamsByName.at("Post gain")->setPos(left2, top2, size);
    mUi->mParamsByName.at("Bass")->setPos(left1, top2, size);
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    mUiReady = true;
  }
};