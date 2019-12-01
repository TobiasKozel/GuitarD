#pragma once

#include "src/node/Node.h"
#include "src/node/NodeUi.h"


class InputNodeUi final : public NodeUi {
  
  iplug::igraphics::IText mBlocksizeText;
  string mInfo;
public:
  InputNodeUi(NodeShared* param) : NodeUi(param) {
    mInfo = "";
    mBlocksizeText = DEBUG_FONT;
  }

  void Draw(IGraphics& g) override {
    NodeUi::Draw(g);
    mInfo = "Blocksize: " + to_string(shared->node->mLastBlockSize) + " Sample-Rate: " + to_string(shared->node->mSampleRate);
    g.DrawText(mBlocksizeText, mInfo.c_str(), mRECT);
  }
};

class InputNode final : public Node {
public:
  InputNode(MessageBus::Bus* pBus) : Node() {
    mLastBlockSize = -1;
    if (shared.X == shared.Y && shared.X == 0) {
      // Place it at the screen edge if no position is set
      shared.Y = PLUG_HEIGHT * 0.5;
      shared.X = shared.width * 0.3;
    }
    setup(pBus, 48000, MAX_BUFFER, 2, 0, 1);
  }

  void ProcessBlock(int) {}

  void CopyIn(iplug::sample** in, int nFrames) {
    mLastBlockSize = nFrames;
    for (int c = 0; c < mChannelCount; c++) {
      for (int i = 0; i < nFrames; i++) {
        mBuffersOut[0][c][i] = in[c][i];
      }
    }
    mIsProcessed = true;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    shared.graphics = pGrahics;
    mUi = new InputNodeUi(&shared);
    mUi->setColor(IColor(255, 100, 150, 100));
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    mUiReady = true;
  }
};