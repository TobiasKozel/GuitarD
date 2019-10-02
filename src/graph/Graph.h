#pragma once
#include "IPlugConstants.h"
#include "src/graph/Node.h"
#include "src/graph/DummyNode.h"
#include "src/graph/nodes/simple_delay/SimpleDelayNode.h"

#define MAXNODES 128

class Graph {
  Node** nodes;
  DummyNode* input;
  DummyNode* output;
  int nodeCount;
  int channelCount;
public:
  Graph(int p_sampleRate, int p_channles = 2) {
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
    testAdd();
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
    nodes[0] = new SimpleDelayNode(sampleRate);
    nodes[0]->inputs[0] = input;
    output->inputs[0] = nodes[0];
  }

private:
  int sampleRate;
};