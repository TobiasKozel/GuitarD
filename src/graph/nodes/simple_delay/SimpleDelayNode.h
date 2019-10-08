#pragma once
#include "src/graph/Node.h"
#include "SimpleDelay.h"

class SimpleDelayNode : public Node {
  SimpleDelay delay;
public:
  SimpleDelayNode(int p_samplerate, ParameterManager* p_manager, int p_maxBuffer = 512, int p_channles = 2)
    : Node(p_manager, p_samplerate, p_maxBuffer, 1, 1, 2) {

    paramsFromFaust(&delay);
    type = "StereoToolNode";
  }

  void ProcessBlock(int nFrames) {
    for (int i = 0; i < parameterCount; i++) {
      parameters[i]->update();
    }
    delay.compute(nFrames, inputs[0]->outputs[0], outputs[0]);
  }
};
