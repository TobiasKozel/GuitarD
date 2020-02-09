#pragma once
#include "../../node/Node.h"

namespace guitard {
  class CombineNode final : public Node {
    const sample smoothing = 0.999;
    sample smoothed[8] = { 0 };
    sample pan1 = 0;
    sample pan2 = 0;
    sample mix = 0.5;
    sample mAddMode = 0;
    sample** emptyBuffer;
  public:
    CombineNode(const std::string pType) {
      shared.type = pType;
      shared.width = 200;
      shared.height = 150;
    }

    void ProcessBlock(const int nFrames) override {
      if (mIsProcessed) { return; }
      NodeSocket* s1 = shared.socketsIn[0];
      NodeSocket* s2 = shared.socketsIn[1];

      // see which inputs are connected
      const bool has1 = s1->getConnectedNode() != nullptr;
      const bool has2 = s2->getConnectedNode() != nullptr;
      if (has1 == has2 && has1 == false) {
        outputSilence();
        return;
      }

      if ((has1 && !s1->getConnectedNode()->mIsProcessed) || (has2 && !s2->getConnectedNode()->mIsProcessed)) {
        // skip until inputs are ready
        return;
      }

      // Choose the buffer from the input or use silence
      sample** buffer1 = has1 ? s1->mConnectedTo[0]->mParentBuffer : emptyBuffer;
      sample** buffer2 = has2 ? s2->mConnectedTo[0]->mParentBuffer : emptyBuffer;

      // Update the params
      shared.parameters[0].update();
      shared.parameters[1].update();
      shared.parameters[2].update();
      shared.parameters[3].update();

      // prepare the values
      double baseMix;
      double invMix;
      if (mAddMode > 0.5) {
        baseMix = 1;
        invMix = 1;
      }
      else {
        baseMix = mix;
        invMix = 1 - baseMix;
      }

      const double pan1l = std::min(1.0, std::max(-pan1 + 1.0, 0.0)) * invMix * (1.0 - smoothing);
      const double pan1r = std::min(1.0, std::max(+pan1 + 1.0, 0.0)) * invMix * (1.0 - smoothing);
      const double pan2l = std::min(1.0, std::max(-pan2 + 1.0, 0.0)) * baseMix * (1.0 - smoothing);
      const double pan2r = std::min(1.0, std::max(+pan2 + 1.0, 0.0)) * baseMix * (1.0 - smoothing);

      // do the math
      for (int i = 0; i < nFrames; i++) {
        smoothed[0] = pan1l + smoothed[1] * smoothing;
        smoothed[2] = pan2l + smoothed[3] * smoothing;
        smoothed[4] = pan1r + smoothed[5] * smoothing;
        smoothed[6] = pan2r + smoothed[7] * smoothing;
        mBuffersOut[0][0][i] = buffer1[0][i] * smoothed[0] + buffer2[0][i] * smoothed[2];
        mBuffersOut[0][1][i] = buffer1[1][i] * smoothed[4] + buffer2[1][i] * smoothed[6];
        smoothed[1] = smoothed[0];
        smoothed[3] = smoothed[2];
        smoothed[5] = smoothed[4];
        smoothed[7] = smoothed[6];
      }

      mIsProcessed = true;
    }

    void setup(MessageBus::Bus* pBus, const int pSamplerate = 48000, const int pMaxBuffer = 512, int pChannles = 2, int pInputs = 1, int pOutputs = 1) override {
      Node::setup(pBus, pSamplerate, pMaxBuffer, 2, 2, 1);
      shared.parameters[shared.parameterCount] = ParameterCoupling(
        "PAN 1", &pan1, 0.0, -1.0, 1.0, 0.01
      );
      shared.parameters[shared.parameterCount].x = -40;
      shared.parameters[shared.parameterCount].y = -20;
      shared.parameterCount++;

      shared.parameters[shared.parameterCount] = ParameterCoupling(
        "PAN 2", &pan2, 0.0, -1.0, 1.0, 0.01
      );
      shared.parameters[shared.parameterCount].x = -40;
      shared.parameters[shared.parameterCount].y = 40;
      shared.parameterCount++;

      shared.parameters[shared.parameterCount] = ParameterCoupling(
        "MIX", &mix, 0.5, 0.0, 1.0, 0.01
      );
      shared.parameters[shared.parameterCount].x = 40;
      shared.parameters[shared.parameterCount].y = -20;
      shared.parameterCount++;

      shared.parameters[shared.parameterCount] = ParameterCoupling(
        "Add mode", &mAddMode, 0, 0.0, 1.0, 1.0
      );
      shared.parameters[shared.parameterCount].x = 40;
      shared.parameters[shared.parameterCount].y = 20;
      shared.parameters[shared.parameterCount].wantsDawParameter = false;
      shared.parameterCount++;

      shared.socketsIn[0]->mY += -35;
      shared.socketsIn[1]->mY += -25;
      shared.socketsOut[0]->mY += -10;
    }

    void deleteBuffers() override {
      Node::deleteBuffers();
      if (emptyBuffer != nullptr) {
        for (int c = 0; c < mChannelCount; c++) {
          delete emptyBuffer[c];
        }
        emptyBuffer = nullptr;
      }
    }

    void createBuffers() override {
      Node::createBuffers();
      // this will be used to do processing on a disconnected node
      emptyBuffer = new sample * [mChannelCount];
      for (int c = 0; c < mChannelCount; c++) {
        emptyBuffer[c] = new sample[shared.maxBlockSize];
        for (int i = 0; i < shared.maxBlockSize; i++) {
          emptyBuffer[c][i] = 0;
        }
      }
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::TOOLS);
    }
#endif
  };
}
