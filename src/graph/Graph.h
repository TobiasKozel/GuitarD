#pragma once
#include "IPlugConstants.h"
#include "src/graph/Node.h"
#include "src/graph/DummyNode.h"

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
  }

  void ProcessBlock(iplug::sample** in, iplug::sample** out, int nFrames) {
    input->outputs[0] = in;

    output->ProcessBlock(in, out, nFrames);
  }

private:
  int sampleRate;
};