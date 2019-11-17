#pragma once
#include "BitCrusher.h"

class BitCrusherNodeUi : public NodeUi {
public:
  BitCrusherNodeUi(NodeUiParam param) : NodeUi(param) {
    bgSVG = param.pGraphics->LoadSVG(SVGBITTERBG_FN);
    // setUpDimensions(bgSVG.W(), bgSVG.H());
    ;
  }

  ~BitCrusherNodeUi() {
  }

  void Draw(IGraphics& g) override {
    DrawShadow(g);
    g.DrawSVG(bgSVG, mTargetRECT);
    // g.FillRoundRect(mColor, mTargetRECT);
    DrawHeader(g);
  }

  ISVG bgSVG = ISVG(nullptr);
};


class BitCrusherNode : public BitCrusher {
public:
  BitCrusherNode(std::string pType) {
    type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    mUi = new BitCrusherNodeUi(NodeUiParam{
      mBus,
      pGrahics,
      IColor(255, 100, 150, 100),
      250, 190,
      &X, &Y,
      &parameters,
      &inSockets,
      &outSockets,
      this
      });
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    uiReady = true;
  }
};