#pragma once
#include "../../main/Node.h"

namespace guitard {

  class RectifyNode final : public Node {
    sample mGainUp = 1.0;
    sample mGainDown = 1.0;

  public:
    void setup(int pSamplerate, int pMaxBuffer, int, int, int) override {
      Node::setup(pSamplerate, pMaxBuffer, 1, 1, 2);
      addByPassParam();
      addParameter("Up", &mGainUp, 1.0, -2.0, 2.0, 0.01, {-50, 0});
      addParameter("Down", &mGainDown, 1.0, -2.0, 2.0, 0.01, { 50, 0 });
    }

    void ProcessBlock(int nFrames) override {
      if (byPass()) { return; }
      sample** in = mSocketsIn[0].mBuffer;
      mParameters[1].update();
      mParameters[2].update();
      for (int i = 0; i < nFrames; i++) {
        for (int c = 0; c < mChannelCount; c++) {
          const sample val = in[c][i];
          mSocketsOut[0].mBuffer[c][i] = val * ((val > 0) ? mGainUp : mGainDown);
        }
      }
    }
  };

  GUITARD_REGISTER_NODE(RectifyNode, "Rectifier", "Distortion", "adasdsa")
}