#pragma once
#include "mutex.h"
#include "IPlugConstants.h"
#include "src/graph/Node.h"
#include "src/graph/nodes/DummyNode.h"
#include "src/constants.h"
#include "src/logger.h"
#include "src/graph/ParameterManager.h"
#include "src/graph/nodes/NodeList.h"
#include "thirdparty/json.hpp"
#include "src/graph/ui/Background.h"


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
    // testAdd();
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
    WDBGMSG("x %f y %f s %f\n", x, y, scale);
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

  void serialize(nlohmann::json &serialized) {
    WDL_MutexLock lock(&isProcessing);
    // TODO handle nodes with multiple outputs and figure out which one is connected to the out
    serialized["output"] = { nodes.Find(output->inputs[0]), 0 };
    serialized["nodes"] = nlohmann::json::array();
    for (int i = 0, pos = 0; i < nodes.GetSize(); i++) {
      Node* node = nodes.Get(i);
      if (node != nullptr) {
        serialized["nodes"][pos]["position"] = { node->x, node->y };
        // The index shouldn't really matter since they're all in order
        serialized["nodes"][pos]["idx"] = i;
        serialized["nodes"][pos]["type"] = node->type;
        serialized["nodes"][pos]["inputs"] = nlohmann::json::array();
        for (int prev = 0; prev < node->inputCount; prev++) {
          if (node->inputs[prev] == nullptr) {
            // -2 means not connected
            serialized["nodes"][pos]["inputs"][prev] = -2;
          }
          else if (node->inputs[prev] == input) {
            // -1 means connected to the global input
            serialized["nodes"][pos]["inputs"][prev] = -1;
          }
          else {
            // otherwise just get the index of the actual node
            serialized["nodes"][pos]["inputs"][prev] = nodes.Find(node->inputs[prev]);
          }
        }
        serialized["nodes"][pos]["parameters"] = nlohmann::json::array();
        for (int p = 0; p < node->parameterCount; p++) {
          serialized["nodes"][pos]["parameters"][p] = {
            { "idx", node->parameters[p]->parameterIdx },
            // TODO the type is not atomic and might tear
            // only use this when theres no daw parameter
            { "value", *(node->parameters[p]->value) }
          };
        }
        pos++;
      }
    }
  }

  void deserialize(nlohmann::json& serialized) {
    WDL_MutexLock lock(&isProcessing);
    for (int i = 0; i < nodes.GetSize(); i++) {
      removeNode(i);
    }
    output->inputs[0] = input;
    int expectedIndex = 0;
    // create all the nodes and setup the parameters in the first pass
    for (auto sNode : serialized["nodes"]) {
      std::string className = sNode["type"];
      Node* node = createNode(className);
      if (node == nullptr) { continue; }
      node->setup(&paramManager, sampleRate);
      if (expectedIndex != sNode["idx"]) {
        WDBGMSG("Deserialization mismatched indexes, this will not load right\n");
      }
      nodes.Add(node);
      int paramIdx = 0;
      for (auto param : sNode["parameters"]) {
        if (paramIdx >= node->parameterCount) { break; }
        node->parameters[paramIdx]->parameterIdx = param["idx"];
        *(node->parameters[paramIdx]->value) = param["value"];
        paramIdx++;
      }
      node->claimParameters();
      if (graphics != nullptr && graphics->WindowIsOpen()) {
        node->setupUi(graphics);
      }
      expectedIndex++;
    }

    // link them all up accordingly in the second pass
    int currentNodeIdx = 0;
    for (auto sNode : serialized["nodes"]) {
      int currentInputIdx = 0;
      for (int inNodeIdx : sNode["inputs"]) {
        if (inNodeIdx >= 0 && nodes.Get(inNodeIdx) != nullptr) {
          nodes.Get(currentInputIdx)->inputs[currentInputIdx] = nodes.Get(inNodeIdx);
        }
        else if (inNodeIdx == -1) {
          // thie index is -1 if the node is connected to the global input
          // if it's -2 it's not connected at all and we'll just leave it at a nullptr
          nodes.Get(currentInputIdx)->inputs[currentInputIdx] = input;
        }
        currentInputIdx++;
      }
      currentNodeIdx++;
    }

    int outIndex = serialized["output"][0];
    if (nodes.Get(outIndex) != nullptr) {
      output->inputs[serialized["output"][1]] = nodes.Get(outIndex);
    }
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
