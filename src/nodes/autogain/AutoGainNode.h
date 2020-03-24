#pragma once
#include "../../main/Node.h"

namespace guitard {
  class AutoGainNode final : public Node {
    const sample detectTime = 2.0;
    bool detectMode = false;
    sample gain = 1;
    sample detectedLoudness = 0;
    int samplesPassed = 0;
    int samplesTarget = 0;
  public:
    AutoGainNode() {
      mDimensions.x = 100;
      mDimensions.y = 100;
      addByPassParam();

      mParameters[
        addParameter("Gain", &gain, 0.0, -90.0, 40.0, 0.1)
      ].type = ParameterCoupling::Gain;
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
      if (byPass()) { return; }
      sample** buffer = mSocketsIn[0].mBuffer;
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
#ifndef GUITARD_HEADLESS
          if (mParameters[1].control != nullptr) {

          }
          else {

          }
#endif

          samplesPassed = 0;
          detectMode = false;
        }
      }

      mParameters[1].update();
      sample val = ParameterCoupling::dbToLinear(gain);
      for (int c = 0; c < mChannelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          mSocketsOut[0].mBuffer[c][i] = buffer[c][i] * val;
        }
      }
    }
  };

  GUITARD_REGISTER_NODE(
    AutoGainNode, "Auto Level", "Dynamics", "Intends to set the level automatically"
  )
}
