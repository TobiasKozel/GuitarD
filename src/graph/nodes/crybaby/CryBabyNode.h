#pragma once
#include "src/graph/Node.h"
#include "Crybaby.h"

class CryBabyNode : public Node {
public:
  CryBabyNode() : Node() {
    type = "CryBabyNode";
  }

  void setup(ParameterManager* p_paramManager, int p_samplerate = 48000, int p_maxBuffer = 512, int p_inputs = 1, int p_outputs = 1, int p_channles = 2) {
    Node::setup(p_paramManager, p_samplerate, p_maxBuffer, 1, 1, 2);
    faustmodule = new Crybaby();
    paramsFromFaust();
    parameters[0]->y = 100;
    parameters[0]->x = 100;
  }
};