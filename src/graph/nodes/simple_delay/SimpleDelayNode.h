#pragma once
#include "src/graph/Node.h"
#include "SimpleDelay.h"

class SimpleDelayNode : public Node {
  SimpleDelay delay;
public:
  double *time, *dry, *wet, *decay, *lowpass, *resonance, *pitch;
  SimpleDelayNode(int p_samplerate, int p_maxBuffer = 512, int p_channles = 2)
    : Node(p_samplerate, p_maxBuffer, 1, 1, 2) {
    delay.setup(p_samplerate);
    time = delay.getProperty("Time");
    dry = delay.getProperty("Dry");
    wet = delay.getProperty("Wet");
    decay = delay.getProperty("Decay");
    lowpass = delay.getProperty("Lowpass");
    resonance = delay.getProperty("Resonance");
    pitch = delay.getProperty("Pitch");
  }

  void ProcessBlock(int nFrames) {
    delay.compute(nFrames, inputs[0]->outputs[0], outputs[0]);
  }
};