#pragma once
#include "src/graph/Node.h"
#include "SimpleDrive.h"

class SimpleDriveNode : public Node {
public:
  SimpleDriveNode() : Node() {
    type = "SimpleDriveNode";
  }

  void setup(ParameterManager* p_paramManager, int p_samplerate = 48000, int p_maxBuffer = 512, int p_inputs = 1, int p_outputs = 1, int p_channles = 2) {
    Node::setup(p_paramManager, p_samplerate, p_maxBuffer, 1, 1, 2);
    faustmodule = new SimpleDrive();
    paramsFromFaust();
    parameters[0]->y = 100;
    parameters[0]->x = 100;
    parameters[1]->y = 100;
    parameters[1]->x = 200;
    parameters[2]->y = 180;
    parameters[2]->x = 150;
  }
};