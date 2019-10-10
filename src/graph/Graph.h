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
    // I don't really like the mutex here, but it should only be locked if a change to the
    // processing chain is made, which will cause some artifacts anyways
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
      nodes[0] = new StereoToolNode();
      nodes[0]->setup(&paramManager, sampleRate);
      nodes[0]->claimParameters();
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

  void serialize(nlohmann::json &serialized) {
    // get all the nodes a index, TODO move this over to the constructor since this won't change over lifetime of a node
    for (int i = 0; i < MAXNODES; i++) {
      if (nodes[i] != nullptr) {
        nodes[i]->index = i;
      }
    }
    serialized["output"] = { output->inputs[0]->index, 0 };
    serialized["nodes"] = nlohmann::json::array();
    for (int i = 0, pos = 0; i < MAXNODES; i++) {
      Node* node = nodes[i];
      if (node != nullptr) {
        serialized["nodes"][pos]["position"] = { node->x, node->y };
        serialized["nodes"][pos]["idx"] = i;
        serialized["nodes"][pos]["type"] = node->type;
        serialized["nodes"][pos]["inputs"] = nlohmann::json::array();
        for (int prev = 0; prev < node->inputCount; prev++) {
          serialized["nodes"][pos]["inputs"][prev] = node->inputs[prev]->index;
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
    for (int i = 0; i < MAXNODES; i++) {
      removeNode(i);
    }
    output->inputs[0] = input;
    for (auto sNode : serialized["nodes"]) {
      std::string className = sNode["type"];
      Node* node = createNode(className);
      if (node == nullptr) { continue; }
      node->setup(&paramManager, sampleRate);
      nodes[sNode["idx"]] = node;
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
    }

    for (auto sNode : serialized["nodes"]) {
      int inputIdx = 0;
      for (int inNodeIdx : sNode["inputs"]) {
        int ownIndex = sNode["idx"];
        if (nodes[inNodeIdx] != nullptr && nodes[ownIndex] != nullptr) {
          nodes[ownIndex]->inputs[inputIdx] = inNodeIdx == -1 ? input : nodes[inNodeIdx];
        }
        inputIdx++;
      }
    }

    int outIndex = serialized["output"][0];
    if (nodes[outIndex] != nullptr) {
      output->inputs[serialized["output"][1]] = nodes[outIndex];
    }
  }

  void removeNode(int index) {
    if (nodes[index] == nullptr) { return; }
    if (graphics != nullptr) {
      nodes[index]->cleanupUi(graphics);
    }
    delete nodes[index];
    nodes[index] = nullptr;
  }

private:
  int sampleRate;
};