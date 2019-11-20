#pragma once
#include "BitCrusher.h"
class BitCrusherNode final : public BitCrusher {
public:
  explicit BitCrusherNode(const std::string pType) {
    mType = pType;
  }
#define BITTERW 250
#define BITTERH 240
  void setupUi(iplug::igraphics::IGraphics* pGraphics) override {
    mUi = new NodeUi(NodeUiParam{
      mBus, pGraphics,
      BITTERW, BITTERH,
      &mX, &mY, &mParameters, &mSocketsIn, &mSocketsOut, this
    });
    mUi->setSvg(SVGBITTERBG_FN);
    const float left1 = BITTERW * 0.26 - BITTERW / 2;
    const float left2 = BITTERW * 0.68 - BITTERW / 2;
    const float top1 = BITTERW * 0.35 - BITTERH / 2;
    const float top2 = BITTERW * 0.7 - BITTERH / 2;
    const float size = 60;
    mUi->mParamsByName.at("Bits")->setPos(left1, top1, size, false);
    mUi->mParamsByName.at("Downsampling Factor")->setPos(left2, top1, size, false);
    mUi->mParamsByName.at("Mix")->setPos(left1, top2, size, false);
    pGraphics->AttachControl(mUi);
    mUi->setUp();
    mUiReady = true;
  }
};