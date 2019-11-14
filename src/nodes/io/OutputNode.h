#pragma once

#include "src/node/Node.h"
class OutputNodeUi : public NodeUi {
public:
  OutputNodeUi(NodeUiParam param) : NodeUi(param) {
  }
};


class OutputNode : public Node {
public:
  OutputNode() : Node() {
    setup(0, MAXBUFFER, 2, 1, 0);
  }

  void ProcessBlock(int) {
    NodeSocket* in = inSockets.Get(0)->connectedTo;
    if (in == nullptr) {
      isProcessed = true;
    }
    else {
      isProcessed = in->parentNode->isProcessed;
    }
  }

  void CopyOut(iplug::sample** out, int nFrames) {
    NodeSocket* in = inSockets.Get(0)->connectedTo;
    if (maxBuffer < nFrames || in == nullptr || !in->parentNode->isProcessed) {
      for (int c = 0; c < channelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          out[c][i] = 0;
        }
      }
    }
    else {
      iplug::sample** buf = in->parentBuffer;
      for (int c = 0; c < channelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          out[c][i] = buf[c][i];
        }
      }
    }
    isProcessed = true;
  }

  void OnReset(int p_sampleRate, int p_channels) override {
    channelCount = p_channels;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    if (X == Y && X == 0) {
      // Place it at the screen edge if no position is set
      Y = pGrahics->Height() / 2;
      X = pGrahics->Width();
    }
    mUi = new OutputNodeUi(NodeUiParam{
      pGrahics,
      IColor(255, 100, 150, 100),
      250, 150,
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