#pragma once
#include "src/graph/nodes/NodeList.h"
#include "StereoTool.h"

class StereoToolNode : public Node {
  StereoTool tool;
public:
  StereoToolNode(int p_samplerate, ParameterManager* p_manager, int p_maxBuffer = 512, int p_channles = 2)
    : Node(p_manager, p_samplerate, p_maxBuffer, 1, 1, 2) {

    paramsFromFaust(&tool);
    type = "StereoToolNode";
  }

  void ProcessBlock(int nFrames) {
    for (int i = 0; i < parameterCount; i++) {
      parameters[i]->update();
    }
    tool.compute(nFrames, inputs[0]->outputs[0], outputs[0]);
  }
private:
  static DerivedRegister<StereoToolNode> reg;
};


DerivedRegister<StereoToolNode> StereoToolNode::reg("StereoToolNode");