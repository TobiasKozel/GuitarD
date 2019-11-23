#pragma once

#include "src/node/Node.h"

class OutputNodeUi final : public NodeUi {
public:
  OutputNodeUi(NodeShared* param) : NodeUi(param) {
  }
};


class OutputNode final : public Node {
public:
  OutputNode(MessageBus::Bus* pBus) : Node() {
    setup(pBus, 0, MAX_BUFFER, 2, 1, 0);
  }

  void ProcessBlock(int) {
    NodeSocket* in = shared.socketsIn.Get(0)->mConnectedTo;
    if (in == nullptr) {
      mIsProcessed = true;
    }
    else {
      mIsProcessed = in->mParentNode->mIsProcessed;
    }
  }

  void CopyOut(iplug::sample** out, int nFrames) {
    NodeSocket* in = shared.socketsIn.Get(0)->mConnectedTo;
    if (mMaxBuffer < nFrames || in == nullptr || !in->mParentNode->mIsProcessed) {
      for (int c = 0; c < mChannelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          out[c][i] = 0;
        }
      }
    }
    else {
      iplug::sample** buf = in->mParentBuffer;
      for (int c = 0; c < mChannelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          out[c][i] = buf[c][i];
        }
      }
    }
    mIsProcessed = true;
  }

  void OnReset(int p_sampleRate, int p_channels) override {
    mChannelCount = p_channels;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    if (shared.X == shared.Y && shared.X == 0) {
      // Place it at the screen edge if no position is set
      shared.Y = pGrahics->Height() / 2.f;
      shared.X = pGrahics->Width();
    }
    shared.graphics = pGrahics;
    mUi = new OutputNodeUi(&shared);
    mUi->setColor(IColor(255, 100, 150, 100));
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    mUiReady = true;
  }
};