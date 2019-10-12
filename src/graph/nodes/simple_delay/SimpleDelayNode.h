#pragma once
#include "src/graph/Node.h"
#include "SimpleDelay.h"

class SimpleDelayNode : public Node {
public:
  SimpleDelayNode() : Node() {
    type = "SimpleDelayNode";
  }

  void setup(ParameterManager* p_paramManager, int p_samplerate = 48000, int p_maxBuffer = 512, int p_inputs = 1, int p_outputs = 1, int p_channles = 2) {
    Node::setup(p_paramManager, p_samplerate, p_maxBuffer, 1, 1, 2);
    faustmodule = new SimpleDelay();
    paramsFromFaust();
    for (int i = 0; i < parameterCount; i++) {
      parameters[i]->y = i * 80;
    }
  }

};
