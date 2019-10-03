#pragma once
#include "src/graph/Node.h"
#include "SimpleDelay.h"

class SimpleDelayNode : public Node {
  SimpleDelay delay;
public:
  // double* time, * dry, * wet, * decay, * lowpass, * resonance, * pitch;
  SimpleDelayNode(int p_samplerate, ParameterManager* p_manager, int p_maxBuffer = 512, int p_channles = 2)
    : Node(p_samplerate, p_maxBuffer, 1, 1, 2) {

    paramManager = p_manager;
    parameters = new ParameterCoupling*[delay.ui.properties.size()];
    delay.setup(p_samplerate);
    parameterCount = 0;
    for (auto p : delay.ui.properties) {
      auto name = p.first;
      auto val = p.second;
      parameters[parameterCount] = paramManager->claimParameter(val, name);
      parameterCount++;
    }
    
    //time = delay.getProperty("Time");
    //dry = delay.getProperty("Dry");
    //wet = delay.getProperty("Wet");
    //decay = delay.getProperty("Decay");
    //lowpass = delay.getProperty("Lowpass");
    //resonance = delay.getProperty("Resonance");
    //pitch = delay.getProperty("Pitch");
  }

  ~SimpleDelayNode() {
    for (int i = 0; i < parameterCount; i++) {
      paramManager->releaseParameter(parameters[i]);
    }
    delete parameters;
  }

  void ProcessBlock(int nFrames) {
    for (int i = 0; i < parameterCount; i++) {
      parameters[i]->update();
    }
    delay.compute(nFrames, inputs[0]->outputs[0], outputs[0]);
  }
};