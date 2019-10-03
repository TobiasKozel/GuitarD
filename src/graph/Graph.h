#pragma once
#include "IPlugConstants.h"
#include "src/graph/Node.h"
#include "src/graph/nodes/DummyNode.h"
#include "src/constants.h"
#include "src/graph/ParameterManager.h"

#include "src/graph/nodes/simple_delay/SimpleDelayNode.h"

class Graph {
public:
  Node** nodes;
  DummyNode* input;
  DummyNode* output;
  int nodeCount;
  int channelCount;
  ParameterManager* paramManager;

  Graph(ParameterManager* p_paramManager, int p_sampleRate, int p_channles = 2) {
    paramManager = p_paramManager;
    sampleRate = p_sampleRate;
    channelCount = p_channles;
    nodes = new Node*[MAXNODES];
    for (int i = 0; i < MAXNODES; i++) {
      nodes[i] = nullptr;
    }
    nodeCount = 0;
    input = new DummyNode();
    output = new DummyNode();
    output->channelCount = channelCount;
    output->inputs[0] = input;
    // testAdd();
  }

  void ProcessBlock(iplug::sample** in, iplug::sample** out, int nFrames) {
    input->outputs[0] = in;
    for (int i = 0; i < MAXNODES; i++) {
      if (nodes[i] != nullptr) {
        nodes[i]->ProcessBlock(nFrames);
      }
    }
    output->CopyOut(out, nFrames);
  }

  void testAdd() {
    nodes[0] = new SimpleDelayNode(sampleRate, paramManager);
    nodes[0]->inputs[0] = input;
    output->inputs[0] = nodes[0];
  }

private:
  int sampleRate;
};