#pragma once
#include "mutex.h"
#include "IPlugConstants.h"
#include "src/graph/Node.h"
#include "src/graph/nodes/DummyNode.h"
#include "src/constants.h"
#include "src/logger.h"
#include "thirdparty/json.hpp"
#include "src/graph/ui/Background.h"
#include "src/graph/misc/Serializer.h"
#include "src/graph/misc/ParameterManager.h"


class Graph {
  iplug::igraphics::IGraphics* graphics;
public:
  /** Holds all the nodes in the processing graph */
  // Node** nodes;
  WDL_PtrList<Node> nodes;

  /** Dummy nodes to get the audio blocks in and out of the graph */
  DummyNode* input;
  DummyNode* output;
  int channelCount;
  ParameterManager paramManager;
  WDL_Mutex isProcessing;


  Graph(int p_sampleRate, int p_channles = 2) {
    graphics = nullptr;
    sampleRate = p_sampleRate;
    channelCount = p_channles;
    input = new DummyNode();
    output = new DummyNode();
    output->channelCount = channelCount;
    output->inputs[0] = input;
  }

  void ProcessBlock(iplug::sample** in, iplug::sample** out, int nFrames) {
    /**
     * I don't really like the mutex here, but it should only be locked if a change to the
     * processing chain is made, which will cause some artifacts anyways
     */
    WDL_MutexLock lock(&isProcessing);
    input->outputs[0] = in;
    input->isProcessed = true;
    int nodeCount = nodes.GetSize();
    for (int n = 0; n < nodeCount; n++) {
      nodes.Get(n)->isProcessed = false;
    }

    // TODO multiple passes to ensure all the nodes are computed is super dumb
    while (!output->inputs[0]->isProcessed) {
      for (int n = 0; n < nodeCount; n++) {
        nodes.Get(n)->ProcessBlock(nFrames);
      }
    }
    output->CopyOut(out, nFrames);
  }

  void testAdd() {
    WDL_MutexLock lock(&isProcessing);
    if (nodes.GetSize() == 0) {
      Node* node = new StereoToolNode();
      node->setup(&paramManager, sampleRate);
      node->claimParameters();
      node->setupUi(graphics);
      node->inputs[0] = input;
      output->inputs[0] = node;
      nodes.Add(node);
    }
    else {
      output->inputs[0] = input;
      removeNode(0);
    }
  }

  /** The graph needs to know about the graphics context to add and remove the controlls for the nodes */
  void setupUi(iplug::igraphics::IGraphics* pGraphics = nullptr) {
    pGraphics->AttachControl(new Background(pGraphics, [&](float x, float y, float scale) {
      this->onViewPortChange(x, y, scale);
    }));

    if (pGraphics != nullptr && pGraphics != graphics) {
      WDBGMSG("Graphics context changed");
      graphics = pGraphics;
    }
    for (int n = 0; n < nodes.GetSize(); n++) {
        nodes.Get(n)->setupUi(pGraphics);
    }
  }

  void cleanupUi() {
    for (int n = 0; n < nodes.GetSize(); n++) {
      nodes.Get(n)->cleanupUi(graphics);
    }
  }

  void onViewPortChange(float x, float y, float scale) {
    for (int i = 0; i < nodes.GetSize(); i++) {
      nodes.Get(i)->translate(x, y);
    }
    // WDBGMSG("x %f y %f s %f\n", x, y, scale);
  }

  void layoutUi(iplug::igraphics::IGraphics* pGraphics = nullptr) {
    if (pGraphics != nullptr && pGraphics != graphics) {
      WDBGMSG("Graphics context changed");
      graphics = pGraphics;
      // Todo find out whether the context ever changes
    }
    for (int n = 0; n < nodes.GetSize(); n++) {
      nodes.Get(n)->layoutChanged();
    }
  }

  void serialize(nlohmann::json &json) {
    WDL_MutexLock lock(&isProcessing);
    json["output"] = { nodes.Find(output->inputs[0]), 0 };
    serializer::serialize(json, nodes, input);

  }

  void deserialize(nlohmann::json& json) {
    WDL_MutexLock lock(&isProcessing);
    for (int i = 0; i < nodes.GetSize(); i++) {
      removeNode(i);
    }
    serializer::deserialize(json, nodes, output, input, sampleRate, &paramManager, graphics);
  }

  void removeNode(int index) {
    Node* node = nodes.Get(index);
    if (node != nullptr) {
      node->cleanupUi(graphics);
      nodes.DeletePtr(node, true);
    }
  }

private:
  int sampleRate;
};
