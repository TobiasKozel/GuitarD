#pragma once
#include "../../node/Node.h"

namespace guitard {

  class RectifyNode final : public Node {
    sample mGainUp = 1.0;
    sample mGainDown = 1.0;

  public:
    RectifyNode(NodeList::NodeInfo* info) {
      shared.info = info;
    }

    void setup(MessageBus::Bus* pBus, int pSamplerate, int pMaxBuffer, int pChannels = 2, int pInputs = 1, int pOutputs = 1) override {
      Node::setup(pBus, pSamplerate, pMaxBuffer, pChannels, pInputs, pOutputs);
      addByPassParam();
      shared.parameters[shared.parameterCount] = ParameterCoupling(
        "Up", &mGainUp,1.0, -2.0, 2.0, 0.01
      );
      shared.parameters[shared.parameterCount].x = -50;
      shared.parameterCount++;

      shared.parameters[shared.parameterCount] = ParameterCoupling(
        "Down", &mGainDown, 1.0, -2.0, 2.0, 0.01
      );
      shared.parameters[shared.parameterCount].x = 50;
      shared.parameterCount++;
    }

    void ProcessBlock(int nFrames) override {
      if (!inputsReady() || mIsProcessed || byPass()) { return; }
      sample** in = shared.socketsIn[0]->mConnectedTo[0]->mParentBuffer;
      shared.parameters[1].update();
      shared.parameters[2].update();
      for (int i = 0; i < nFrames; i++) {
        for (int c = 0; c < mChannelCount; c++) {
          const sample val = in[c][i];
          mBuffersOut[0][c][i] = val * ((val > 0) ? mGainUp : mGainDown);
        }
      }
      mIsProcessed = true;
    }
  };
}