#pragma once

#include "src/graph/Node.h"

class OutputNode : public Node {
public:
  OutputNode(int channels) : Node() {
    setup(0, MAXBUFFER, channels, 1, 0);
  }

  void ProcessBlock(int) {
    NodeSocket* in = inSockets.Get(0)->connectedTo;
    if (in == nullptr) {
      isProcessed = true;
    }
    else {
      isProcessed = in->parentNode->isProcessed;
    }
  }

  void CopyOut(iplug::sample** out, int nFrames) {
    if (maxBuffer < nFrames || inSockets.Get(0)->connectedTo == nullptr) {
      for (int c = 0; c < channelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          out[c][i] = 0;
        }
      }
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