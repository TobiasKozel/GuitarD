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
    NodeSocket* in = inSockets.Get(0)->connectedTo;
    if (maxBuffer < nFrames || in == nullptr || !in->parentNode->isProcessed) {
      for (int c = 0; c < channelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          out[c][i] = 0;
        }
      }
    }
    else {
      iplug::sample** buf = in->parentBuffer;
      for (int c = 0; c < channelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          out[c][i] = buf[c][i];
        }
      }
    }
    isProcessed = true;
  }
};