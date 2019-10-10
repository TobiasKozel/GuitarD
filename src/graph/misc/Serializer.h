#pragma once
#include "thirdparty/json.hpp"
#include "src/graph/Node.h"
#include "src/graph/nodes/NodeList.h"
#include "src/graph/misc/ParameterManager.h"
#include "IGraphics.h"


namespace serializer {
  void serialize(nlohmann::json& serialized, WDL_PtrList<Node>& nodes, Node* input) {
    // TODO handle nodes with multiple outputs and figure out which one is connected to the out
    
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

  void deserialize(
    nlohmann::json& serialized, WDL_PtrList<Node>& nodes, Node* output, Node* input, int sampleRate,
    ParameterManager* paramManager, iplug::igraphics::IGraphics* graphics
  ) {
    output->inputs[0] = input;
    int expectedIndex = 0;
    // create all the nodes and setup the parameters in the first pass
    for (auto sNode : serialized["nodes"]) {
      std::string className = sNode["type"];
      Node* node = createNode(className);
      if (node == nullptr) { continue; }
      node->setup(paramManager, sampleRate);
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

    // connect the output nodes to the global output
    int outIndex = serialized["output"][0];
    if (nodes.Get(outIndex) != nullptr) {
      output->inputs[serialized["output"][1]] = nodes.Get(outIndex);
    }
  }
}

