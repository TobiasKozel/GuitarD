#pragma once
#include "src/node/Node.h"

namespace guitard {
  class AutoGainNode final : public Node {
    const iplug::sample detectTime = 2.0;
    bool detectMode = false;
    iplug::sample gain = 1;
    iplug::sample detectedLoudness = 0;
    int samplesPassed = 0;
    int samplesTarget = 0;
  public:
    AutoGainNode(const std::string pType) {
      shared.type = pType;
      shared.width = 100;
      shared.height = 100;
      addByPassParam();

      shared.parameters[shared.parameterCount] = ParameterCoupling(
        "Gain", &gain, 0.0, -90.0, 40.0, 0.1
      );
      shared.parameters[shared.parameterCount].type = ParameterCoupling::Gain;
      shared.parameterCount++;
    }

    void startDetectMode() {
      detectedLoudness = 0;
      detectMode = true;
    }

    virtual void OnSamplerateChanged(int pSampleRate) override {
      samplesTarget = pSampleRate * detectTime;
      mSampleRate = pSampleRate;
    }

    void ProcessBlock(const int nFrames) override {
      if (!inputsReady() || mIsProcessed || byPass()) { return; }
      sample** buffer = shared.socketsIn[0]->mConnectedTo[0]->mParentBuffer;
      double avg = 0;
      for (int c = 0; c < mChannelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          avg += buffer[c][i] * buffer[c][i];
        }
      }
      avg /= nFrames * mChannelCount;
      if (detectMode) {
        if (samplesPassed < samplesTarget) {
          samplesPassed += nFrames;
          detectedLoudness += avg;
        }
        else {
          if (shared.parameters[1].control != nullptr) {

          }
          else {

          }

          samplesPassed = 0;
          detectMode = false;
        }
      }

      shared.parameters[1].update();
      sample val = ParameterCoupling::dbToLinear(gain);
      for (int c = 0; c < mChannelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          mBuffersOut[0][c][i] = buffer[c][i] * val;
        }
      }
      mIsProcessed = true;
    }

    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      Node::setupUi(pGrahics);
      mUi->setColor(Theme::Categories::DYNAMICS);
    }
  };
}