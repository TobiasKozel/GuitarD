#pragma once

#include "../../node/Node.h"
namespace guitard {

  class InputNode final : public Node {
  public:
    InputNode() : Node() {
      mInfo = new NodeList::NodeInfo{ "InputNode", "Input" };
      mLastBlockSize = -1;
#ifndef GUITARD_HEADLESS
      if (mPos.x == mPos.y && mPos.x == 0) {
        // Place it at the screen edge if no position is set
        mPos.y = PLUG_HEIGHT * 0.5f;
        mPos.x = mDimensions.x * 0.3f;
      }
#endif
      Node::setup(48000, GUITARD_MAX_BUFFER, 0, 1, 2);
    }

    ~InputNode() {
      delete mInfo;
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
          mSocketsOut[0].mBuffer[0][i] = in[0][i];
          mSocketsOut[0].mBuffer[1][i] = in[0][i];
        }
      }
      else {
        for (int i = 0; i < nFrames; i++) {
          for (int c = 0; c < mChannelCount; c++) {
            mSocketsOut[0].mBuffer[c][i] = in[c][i];
          }
        }
      }
    }

  private:
    int mInputChannels = 2;
  };
}

#ifndef GUITARD_HEADLESS
#include "../../ui/NodeUi.h"
namespace guitard {
  class InputNodeUi final : public NodeUi {

    iplug::igraphics::IText mBlocksizeText;
    String mInfo;
  public:
    InputNodeUi(Node* node, MessageBus::Bus* bus) : NodeUi(node, bus) {
      mInfo = "";
      mBlocksizeText = DEBUG_FONT;
    }

    void Draw(IGraphics& g) override {
      NodeUi::Draw(g);
      mInfo = "Blocksize: " + std::to_string(mNode->mLastBlockSize) + " Sample-Rate: " + std::to_string(mNode->mSampleRate);
      g.DrawText(mBlocksizeText, mInfo.c_str(), mRECT);
    }
  };

  GUITARD_REGISTER_NODE_UI(InputNode, InputNodeUi)
}
#endif
