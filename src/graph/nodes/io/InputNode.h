#pragma once

#include "src/graph/Node.h"

class InputNode : public Node {
public:
  InputNode(int channels) : Node() {
    setup(0, MAXBUFFER, channels, 0, 1);
  }

  void ProcessBlock(int) {}

  void CopyIn(iplug::sample** in, int nFrames) {
    for (int c = 0; c < channelCount; c++) {
      for (int i = 0; i < nFrames; i++) {
        outputs[0][c][i] = in[c][i];
      }
    }
    isProcessed = true;
  }
};