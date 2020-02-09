#pragma once

#include "../../node/Node.h"
#include "../../node/NodeUi.h"

namespace guitard {
#ifndef GUITARD_HEADLESS
  class InputNodeUi final : public NodeUi {

    iplug::igraphics::IText mBlocksizeText;
    std::string mInfo;
  public:
    InputNodeUi(NodeShared* param) : NodeUi(param) {
      mInfo = "";
      mBlocksizeText = DEBUG_FONT;
    }

    void Draw(IGraphics& g) override {
      NodeUi::Draw(g);
      mInfo = "Blocksize: " + std::to_string(shared->node->mLastBlockSize) + " Sample-Rate: " + std::to_string(shared->node->mSampleRate);
      g.DrawText(mBlocksizeText, mInfo.c_str(), mRECT);
    }
  };
#endif

  class InputNode final : public Node {
  public:
    InputNode(MessageBus::Bus* pBus) : Node() {
      shared.type = "Input";
      mLastBlockSize = -1;
#ifndef GUITARD_HEADLESS
      if (shared.X == shared.Y && shared.X == 0) {
        // Place it at the screen edge if no position is set
        shared.Y = PLUG_HEIGHT * 0.5;
        shared.X = shared.width * 0.3;
      }
#endif
      setup(pBus, 48000, MAX_BUFFER, 2, 0, 1);
    }

    void ProcessBlock(int) override {}

    void setInputChannels(int pInputChannels = 2) {
      if (pInputChannels != mInputChannels) {
        mInputChannels = pInputChannels;
      }
    }

    /**
     * Puts the given buffer into the node
     * Basically used to inject outside audio into the graph
     */
    void CopyIn(sample** in, int nFrames) {
      mLastBlockSize = nFrames;
      if (mInputChannels == 1) {
        for (int i = 0; i < nFrames; i++) {
          mBuffersOut[0][0][i] = in[0][i];
          mBuffersOut[0][1][i] = in[0][i];
        }
      }
      else {
        for (int c = 0; c < mChannelCount; c++) {
          for (int i = 0; i < nFrames; i++) {
            mBuffersOut[0][c][i] = in[c][i];
          }
        }
      }
      mIsProcessed = true;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      shared.graphics = pGrahics;
      mUi = new InputNodeUi(&shared);
      mUi->setColor(IColor(255, 100, 150, 100));
      pGrahics->AttachControl(mUi);
      mUi->setUp();
      mUiReady = true;
    }
#endif
  private:
    int mInputChannels = 2;
  };

}