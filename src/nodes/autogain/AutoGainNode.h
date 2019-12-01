#pragma once
#include "src/node/Node.h"


class AutoGainNode final : public Node {
  const double detectTime = 2.0;
  bool detectMode = false;
  double gain = 1;
  double detectedLoudness = 0;
  int samplesPassed = 0;
  int samplesTarget = 0;
public:
  AutoGainNode(const std::string pType) {
    mType = pType;
    shared.width = 300;
    shared.height = 300;
    addByPassParam();

    ParameterCoupling* p = new ParameterCoupling(
      "Gain", &gain, 0.0, -130.0, 60.0, 0.1
    );
    p->type = ParameterCoupling::Gain;

    shared.parameters[shared.parameterCount] = p;
    shared.parameterCount++;
  }

  void startDetectMode() {
    detectedLoudness = 0;
    detectMode = true;
  }

  virtual void OnSamplerateChanged(int pSampleRate) {
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
        if (shared.parameters[1]->control != nullptr) {
          
        }
        else {
          
        }

        samplesPassed = 0;
        detectMode = false;
      }
    }

    shared.parameters[1]->update();
    gain = ParameterCoupling::dbToLinear(shared.parameters[1]->getValue());
    for (int c = 0; c < mChannelCount; c++) {
      for (int i = 0; i < nFrames; i++) {
        mBuffersOut[0][c][i] = buffer[c][i] * gain;
      }
    }
    mIsProcessed = true;
  }
};