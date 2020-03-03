#pragma once

#include "../../node/Node.h"

namespace guitard {
  class OutputNode final : public Node {
  public:
    OutputNode() : Node() {
      mInfo = new NodeList::NodeInfo{ "OutputNode", "Output" };
#ifndef GUITARD_HEADLESS
      if (mPos.x == mPos.y && mPos.x == 0) {
        // Place it at the screen edge if no position is set
        mPos.y = PLUG_HEIGHT * 0.5f;
        mPos.x = PLUG_WIDTH - mDimensions.x * 0.3f;
      }
#endif
      Node::setup(0, MAX_BUFFER, 1, 0, 2);
    }

    void ProcessBlock(int) override {
      NodeSocket* in = mSocketsIn[0]->mConnectedTo[0];
      if (in == nullptr) {
        mIsProcessed = true;
      }
      else {
        mIsProcessed = in->mParentNode->mIsProcessed;
      }
    }

    void CopyOut(sample** out, int nFrames) {
      NodeSocket* in = mSocketsIn[0]->mConnectedTo[0];
      if (mMaxBlockSize < nFrames || in == nullptr || !in->mParentNode->mIsProcessed) {
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

//#ifndef GUITARD_HEADLESS
//    void setupUi(IGraphics* pGrahics) override {
//      shared.graphics = pGrahics;
//      mUi = new OutputNodeUi(&shared);
//      mUi->setColor(IColor(255, 100, 150, 100));
//      pGrahics->AttachControl(mUi);
//      mUi->setUp();
//      mUiReady = true;
//    }
//#endif
//
//
  };

#ifndef GUITARD_HEADLESS
    class OutputNodeUi final : public NodeUi {
    public:
      OutputNodeUi(Node* node, MessageBus::Bus* bus) : NodeUi(node, bus) { }
    };

    GUITARD_REGISTER_NODE_UI(OutputNode, OutputNodeUi)
#endif
}
