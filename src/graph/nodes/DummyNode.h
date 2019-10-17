#pragma once

#include "src/graph/Node.h"

/**
 * This class is only use as a input/output on the for node graph
*/
class DummyNode : public Node {
public:
  DummyNode(bool isIn, int channels) : Node() {
    if (isIn) {
      setup(0, 512, channels, 0, 1);
    }
    else {
      setup(0, 512, channels, 1, 0);
    }
  }

  ~DummyNode() {
  }

  void ProcessBlock(int) {}

  void CopyIn(iplug::sample** in, int nFrames) {
    // PERFORMANCE this can be avoided by updating the pointer in the outputnode socket instead
    NodeSocket* socket = outSockets.Get(0);
    for (int c = 0; c < channelCount; c++) {
      for (int i = 0; i < nFrames; i++) {
        socket->buffer[c][i] = in[c][i];
      }
    }
    isProcessed = true;
  }

  void CopyOut(iplug::sample** out, int nFrames) {
    for (int c = 0; c < channelCount; c++) {
      for (int i = 0; i < nFrames; i++) {
        out[c][i] = inSockets.Get(0)->buffer[c][i];
      }
    }
  }
};