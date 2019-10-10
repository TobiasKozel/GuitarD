#pragma once
#include "src/graph/Node.h"
#include "SimpleDelay.h"

class SimpleDelayNode : public Node {
  SimpleDelay delay;
public:
  SimpleDelayNode() : Node() {
    type = "SimpleDelayNode";
  }

  void setup(ParameterManager* p_paramManager, int p_samplerate = 48000, int p_maxBuffer = 512, int p_inputs = 1, int p_outputs = 1, int p_channles = 2) {
    Node::setup(p_paramManager, p_samplerate, p_maxBuffer, 1, 1, 2);
    paramsFromFaust(&delay);
  }

  void ProcessBlock(int nFrames) {
    if (isProcessed) { return; }
    for (int i = 0; i < inputCount; i++) {
      if (!inputs[i]->isProcessed) {
        return;
      }
    }
    for (int i = 0; i < parameterCount; i++) {
      parameters[i]->update();
    }
    delay.compute(nFrames, inputs[0]->outputs[0], outputs[0]);
  }
};
