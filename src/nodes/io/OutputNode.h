#pragma once

#include "src/node/Node.h"

namespace guitard {
#ifndef GUITARD_HEADLESS
  class OutputNodeUi final : public NodeUi {
  public:
    OutputNodeUi(NodeShared* param) : NodeUi(param) {
    }
  };
#endif


  class OutputNode final : public Node {
  public:
    OutputNode(MessageBus::Bus* pBus) : Node() {
      shared.type = "Output";
#ifndef GUITARD_HEADLESS
      if (shared.X == shared.Y && shared.X == 0) {
        // Place it at the screen edge if no position is set
        shared.Y = PLUG_HEIGHT * 0.5;
        shared.X = PLUG_WIDTH - shared.width * 0.3;
      }
#endif
      setup(pBus, 0, MAX_BUFFER, 2, 1, 0);
    }

    void ProcessBlock(int) override {
      NodeSocket* in = shared.socketsIn[0]->mConnectedTo[0];
      if (in == nullptr) {
        mIsProcessed = true;
      }
      else {
        mIsProcessed = in->mParentNode->mIsProcessed;
      }
    }

    void CopyOut(sample** out, int nFrames) {
      NodeSocket* in = shared.socketsIn[0]->mConnectedTo[0];
      if (shared.maxBlockSize < nFrames || in == nullptr || !in->mParentNode->mIsProcessed) {
        for (int c = 0; c < mChannelCount; c++) {
          for (int i = 0; i < nFrames; i++) {
            // output silence
            out[c][i] = 0;
          }
        }
      }
      else {
        sample** buf = in->mParentBuffer;
        for (int c = 0; c < mChannelCount; c++) {
          for (int i = 0; i < nFrames; i++) {
            out[c][i] = buf[c][i];
          }
        }
      }
      mIsProcessed = true;
    }

    void OnReset(int p_sampleRate, int p_channels, bool force = false) override {
      mChannelCount = p_channels;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(IGraphics* pGrahics) override {
      shared.graphics = pGrahics;
      mUi = new OutputNodeUi(&shared);
      mUi->setColor(IColor(255, 100, 150, 100));
      pGrahics->AttachControl(mUi);
      mUi->setUp();
      mUiReady = true;
    }
#endif
  };
}
