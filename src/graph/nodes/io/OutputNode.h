#pragma once

#include "src/graph/Node.h"

class OutputNode : public Node {
public:
  OutputNode(int channels) : Node() {
    setup(0, MAXBUFFER, channels, 1, 0);
  }

  void ProcessBlock(int) {}

  void CopyOut(iplug::sample** out, int nFrames) {
    if (!inputsReady()) { return; }
    if (maxBuffer < nFrames) {
      outputSilence();
      return;
    }
    iplug::sample** buf = inSockets.Get(0)->connectedTo->parentBuffer;
    for (int c = 0; c < channelCount; c++) {
      for (int i = 0; i < nFrames; i++) {
        out[c][i] = buf[c][i];
      }
    }
    isProcessed = true;
  }
};