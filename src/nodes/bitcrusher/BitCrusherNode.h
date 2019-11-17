#pragma once
#include "BitCrusher.h"

class BitCrusherNodeUi : public NodeUi {
public:
  BitCrusherNodeUi(NodeUiParam param) : NodeUi(param) {
  }

  ~BitCrusherNodeUi() {
  }
};


class BitCrusherNode : public BitCrusher {
public:
  BitCrusherNode(std::string pType) {
    type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    mUi = new BitCrusherNodeUi(NodeUiParam{
      mBus, pGrahics,
      250, 240,
      &X, &Y, &parameters, &inSockets, &outSockets, this
    });
    mUi->setSvg(SVGBITTERBG_FN);
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    uiReady = true;
  }
};