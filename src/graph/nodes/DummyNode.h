#pragma once

#include "src/graph/Node.h"

/**
 * This class is only use as a input/output on the for node graph
*/
class DummyNode : public Node {
public:
  DummyNode() : Node() {
    isProcessed = true;
    // Node::setup(0, 0, 1, 1, 1);
    NodeSocket* in = new NodeSocket(0);
    inSockets.Add(in);
    NodeSocket* out = new NodeSocket(0, this);
    outSockets.Add(out);
    inputCount = 1;
    outputCount = 1;
  }

  void ProcessBlock(int) {}

  void SetIn(iplug::sample** in) {
    NodeSocket* socket = outSockets.Get(0);
    socket->buffer = in;
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