#pragma once
#include "src/graph/Node.h"
#include "SimpleDelay.h"

class SimpleDelayNode : public Node {
  SimpleDelay delay;
public:
  SimpleDelayNode(int p_samplerate, ParameterManager* p_manager, int p_maxBuffer = 512, int p_channles = 2)
    : Node(p_samplerate, p_maxBuffer, 1, 1, 2) {

    paramManager = p_manager;

    // This must be executed first since this will gather all the parameters from faust
    delay.setup(p_samplerate);

    // Keep the pointers around in a normal array since this might be faster than iterating over a vector
    parameters = new ParameterCoupling*[delay.ui.params.size()];
    parameterCount = 0;
    for (auto p : delay.ui.params) {
      parameters[parameterCount] = p;
      parameterCount++;
      if (!paramManager->claimParameter(p)) {
        // this means the manager has no free parameters left
        WDBGMSG("Ran out of daw parameters!");
      }
    }
  }

  ~SimpleDelayNode() {
    float test = 0.4;
    // only delete the array, the UI struct in SimpleDelay will delete all the params inside
    for (int i = 0; i < parameterCount; i++) {
      // however the daw parameters still have to be freed so another node can take them if needed
      paramManager->releaseParameter(parameters[i]);
    }
    delete parameters;
    // Node::~Node();
  }

  void ProcessBlock(int nFrames) {
    for (int i = 0; i < parameterCount; i++) {
      parameters[i]->update();
    }
    delay.compute(nFrames, inputs[0]->outputs[0], outputs[0]);
  }
};
