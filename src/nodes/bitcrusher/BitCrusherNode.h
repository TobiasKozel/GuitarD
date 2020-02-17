#pragma once
#include "../../faust/generated/BitCrusher.h"

namespace guitard {
  class BitCrusherNode final : public FaustGenerated::BitCrusher {
  public:
    explicit BitCrusherNode(NodeList::NodeInfo info) {
      shared.info = info;
      shared.width = 250;
      shared.height = 240;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGraphics) override {
      shared.graphics = pGraphics;
      mUi = new NodeUi(&shared);
      mUi->setSvg(SVGBITTERBG_FN);
      const float left1 = shared.width * 0.26 - shared.width / 2;
      const float left2 = shared.width * 0.68 - shared.width / 2;
      const float top1 = shared.width * 0.35 - shared.height / 2;
      const float top2 = shared.width * 0.7 - shared.height / 2;
      const float size = 60;
      mUi->mParamsByName.at("Bits")->setPos(left1, top1, size, false);
      mUi->mParamsByName.at("Downsampling Factor")->setPos(left2, top1, size, false);
      mUi->mParamsByName.at("Mix")->setPos(left1, top2, size, false);
      pGraphics->AttachControl(mUi);
      mUi->setUp();
      mUiReady = true;
    }
#endif
  };
}