#pragma once
#include "IPlugConstants.h"
#include "src/graph/Node.h"
#include "src/graph/nodes/DummyNode.h"
#include "src/constants.h"
#include "src/logger.h"
#include "src/graph/ParameterManager.h"
#include "mutex.h"

#include "src/graph/nodes/simple_delay/SimpleDelayNode.h"
#include "src/graph/nodes/stereo_tool/StereoToolNode.h"

class Graph {
  iplug::igraphics::IGraphics* graphics;
public:
  /** Holds all the nodes in the processing graph */
  Node** nodes;

  /** Dummy nodes to get the audio blocks in and out of the graph */
  DummyNode* input;
  DummyNode* output;
  int nodeCount;
  int channelCount;
  ParameterManager paramManager;
  WDL_Mutex isProcessing;


  Graph(int p_sampleRate, int p_channles = 2) {
    graphics = nullptr;
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
    WDL_MutexLock lock(&isProcessing);
    input->outputs[0] = in;
    // this is dumb, use the WDL_Pointer list instead
    for (int i = 0; i < MAXNODES; i++) {
      if (nodes[i] != nullptr) {
        nodes[i]->ProcessBlock(nFrames);
      }
    }
    output->CopyOut(out, nFrames);
  }

  void testAdd() {
    WDL_MutexLock lock(&isProcessing);
    if (nodes[0] == nullptr) {
      nodes[0] = new StereoToolNode(sampleRate, &paramManager);
      nodes[0]->setupUi(graphics);
      nodes[0]->inputs[0] = input;
      output->inputs[0] = nodes[0];
    }
    else {
      output->inputs[0] = input;
      nodes[0]->cleanupUi(graphics);
      delete nodes[0];
      nodes[0] = nullptr;
    }
  }

  /** The graph needs to know about the graphics context to add and remove the controlls for the nodes*/
  void setupUi(iplug::igraphics::IGraphics* pGraphics = nullptr) {
    if (pGraphics != nullptr && pGraphics != graphics) {
      WDBGMSG("Graphics context changed");
      graphics = pGraphics;
    }
    for (int i = 0; i < MAXNODES; i++) {
      if (nodes[i] != nullptr) {
        nodes[i]->setupUi(pGraphics);
      }
    }
  }

  void cleanupUi() {
    for (int i = 0; i < MAXNODES; i++) {
      if (nodes[i] != nullptr) {
        nodes[i]->cleanupUi(graphics);
      }
    }
  }

  void layoutUi(iplug::igraphics::IGraphics* pGraphics = nullptr) {
    if (pGraphics != nullptr && pGraphics != graphics) {
      WDBGMSG("Graphics context changed");
      graphics = pGraphics;
      // Todo find out whether the context ever changes
    }
    for (int i = 0; i < MAXNODES; i++) {
      if (nodes[i] != nullptr) {
        nodes[i]->layoutChanged();
      }
    }
  }

private:
  int sampleRate;
};