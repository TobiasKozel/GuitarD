#pragma once
#include "src/graph/Node.h"
#include "StereoTool.h"


class StereoToolNode : public Node {
  StereoTool tool;
public:
  StereoToolNode() : Node() {
    type = "StereoToolNode";
  }

  void setup(ParameterManager* p_paramManager, int p_samplerate = 48000, int p_maxBuffer = 512, int p_inputs = 1, int p_outputs = 1, int p_channles = 2) {
    Node::setup(p_paramManager, p_samplerate, p_maxBuffer, 1, 1, 2);
    paramsFromFaust(&tool);
  }

  void ProcessBlock(int nFrames) {
    for (int i = 0; i < parameterCount; i++) {
      parameters[i]->update();
    }
    tool.compute(nFrames, inputs[0]->outputs[0], outputs[0]);
  }
};

