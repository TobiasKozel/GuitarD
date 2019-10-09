#pragma once

#include "src/graph/Node.h"

/**
 * This class is only use as a input/output on the for node graph
*/
class DummyNode : public Node {
public:
  DummyNode() : Node() {
    isProcessed = true;
    Node::setup(nullptr, 0, 0, 1, 0, 0);
  }

  void ProcessBlock(int) {}

  void CopyOut(iplug::sample** out, int nFrames) {
    if (inputCount && inputs[0] != nullptr) {
      for (int c = 0; c < channelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          // TODO THIS IS BAD SINCE THE OUTPUT OF THE PREV NODE COULD ALSO BE INDEX 1 OR HIGHER
          out[c][i] = inputs[0]->outputs[0][c][i];
        }
      }
    }
  }
};