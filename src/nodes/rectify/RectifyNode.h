#pragma once
#include "../../node/Node.h"

namespace guitard {

  class RectifyNode final : public Node {
    sample mGainUp = 1.0;
    sample mGainDown = 1.0;

  public:
    RectifyNode(NodeList::NodeInfo* info) {
      mInfo = info;
    }

    void setup(int pSamplerate, int pMaxBuffer, int pInputs = 1, int pOutputs = 1, int pChannels = 2) override {
      Node::setup(pSamplerate, pMaxBuffer, pInputs, pOutputs, pChannels);
      addByPassParam();
      addParameter("Up", &mGainUp, 1.0, -2.0, 2.0, 0.01);
      mParameters[mParameterCount].pos.x = -50;

      addParameter("Down", &mGainDown, 1.0, -2.0, 2.0, 0.01);
      mParameters[mParameterCount].pos.x = 50;
    }

    void ProcessBlock(int nFrames) override {
      if (!inputsReady() || mIsProcessed || byPass()) { return; }
      sample** in = mSocketsIn[0]->mConnectedTo[0]->mParentBuffer;
      mParameters[1].update();
      mParameters[2].update();
      for (int i = 0; i < nFrames; i++) {
        for (int c = 0; c < mChannelCount; c++) {
          const sample val = in[c][i];
          mBuffersOut[0][c][i] = val * ((val > 0) ? mGainUp : mGainDown);
        }
      }
      mIsProcessed = true;
    }
  };

  GUITARD_REGISTER_NODE(RectifyNode, "Rectifier", "Distortion", "adasdsa", "image")
}