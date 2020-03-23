#pragma once
#include "../../main/Node.h"

namespace guitard {
  class CombineNode final : public Node {
    const sample smoothing = 0.999;
    sample smoothed[8] = { 0 };
    sample pan1 = 0;
    sample pan2 = 0;
    sample mix = 0.5;
    sample mAddMode = 0;
    sample** emptyBuffer = nullptr;
  public:
    CombineNode(NodeList::NodeInfo* info) {
      mInfo = info;
      mDimensions.x = 200;
      mDimensions.y = 150;
    }

    void ProcessBlock(const int nFrames) override {
      // Choose the buffer from the input or use silence
      sample** buffer1 = mSocketsIn[0].mBuffer;
      sample** buffer2 = mSocketsIn[1].mBuffer;

      // Update the params
      mParameters[0].update();
      mParameters[1].update();
      mParameters[2].update();
      mParameters[3].update();

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
        mSocketsOut[0].mBuffer[0][i] = buffer1[0][i] * smoothed[0] + buffer2[0][i] * smoothed[2];
        mSocketsOut[0].mBuffer[1][i] = buffer1[1][i] * smoothed[4] + buffer2[1][i] * smoothed[6];
        smoothed[1] = smoothed[0];
        smoothed[3] = smoothed[2];
        smoothed[5] = smoothed[4];
        smoothed[7] = smoothed[6];
      }

    }

    void setup(const int pSamplerate, const int pMaxBuffer, int, int, int) override {
      Node::setup(pSamplerate, pMaxBuffer, 2, 1, 2);
      addParameter("PAN 1", &pan1, 0.0, -1.0, 1.0, 0.01, {-40, -20});
      addParameter("PAN 2", &pan2, 0.0, -1.0, 1.0, 0.01, { -40, 40 });
      addParameter("MIX", &mix, 0.5, 0.0, 1.0, 0.01, { 40, -20 });
      mParameters[
        addParameter("Add mode", &mAddMode, 0, 0.0, 1.0, 1.0, { 40, 40 })
      ].wantsDawParameter = false;
    }
  };

  GUITARD_REGISTER_NODE(CombineNode,
    "Combine", "Signal Flow", "Will combine two signals"
  )
}
