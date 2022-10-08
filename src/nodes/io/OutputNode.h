#pragma once

#include "../../main/Node.h"

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
      Node::setup(0, GUITARD_MAX_BUFFER, 1, 0, 2);
    }

    void ProcessBlock(int) override { }

    void CopyOut(sample** out, int nFrames) const {
      for (int c = 0; c < mChannelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          out[c][i] = mSocketsIn[0].mBuffer[c][i];
        }
      }
    }

    void OnReset(int p_sampleRate, int p_channels, bool force = false) override {
      mChannelCount = p_channels;
    }

  };
}

#ifndef GUITARD_HEADLESS
// #include "../../ui/elements/NodeUi.h"
// namespace guitard {
//     class OutputNodeUi final : public NodeUi {
//     public:
//       OutputNodeUi(Node* node, MessageBus::Bus* bus) : NodeUi(node, bus) { }
//     };
//     GUITARD_REGISTER_NODE_UI(OutputNode, OutputNodeUi)
// }
#endif
