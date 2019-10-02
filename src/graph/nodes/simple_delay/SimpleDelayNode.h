#pragma once
#include "src/graph/Node.h"
#include "SimpleDelay.h"

class SimpleDelayNode : public Node {
  SimpleDelay delay;
public:
  double* delayTime;
  SimpleDelayNode(int p_samplerate, int p_maxBuffer = 512, int p_channles = 2)
    : Node(p_samplerate, p_maxBuffer, 1, 1, 2) {
    delay.setup(p_samplerate);
    delayTime = delay.getProperty("delaytime");
  }

  void ProcessBlock(int nFrames) {
    delay.compute(nFrames, inputs[0]->outputs[0], outputs[0]);
  }
};