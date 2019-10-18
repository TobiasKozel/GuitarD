#pragma once
#include "thirdparty/json.hpp"
#include "IGraphics.h"
#include "src/graph/Node.h"
#include "src/graph/misc/NodeList.h"
#include "src/graph/misc/ParameterManager.h"


namespace serializer {
  void serialize(nlohmann::json& serialized, WDL_PtrList<Node>& nodes, Node* input) {
    // TODO handle nodes with multiple outputs and figure out which one is connected to the out
    
    serialized["nodes"] = nlohmann::json::array();
    for (int i = 0, pos = 0; i < nodes.GetSize(); i++) {
      Node* node = nodes.Get(i);
      if (node != nullptr) {
        serialized["nodes"][pos]["position"] = { node->X, node->Y };
        // The index shouldn't really matter since they're all in order
        serialized["nodes"][pos]["idx"] = i;
        serialized["nodes"][pos]["type"] = node->type;
        serialized["nodes"][pos]["inputs"] = nlohmann::json::array();
        for (int prev = 0; prev < node->inputCount; prev++) {
          Node* cNode = node->inSockets.Get(prev)->connectedNode;
          if (cNode == nullptr) {
            // -2 means not connected
            serialized["nodes"][pos]["inputs"][prev] = { -2, 0 };
          }
          else if (cNode == input) {
            // -1 means connected to the global input
            serialized["nodes"][pos]["inputs"][prev] = { -1, 0 };
          }
          else {
            // otherwise just get the index of the actual node
            serialized["nodes"][pos]["inputs"][prev] = { nodes.Find(cNode),
              node->inSockets.Get(prev)->connectedBufferIndex
            };
          }
        }
        serialized["nodes"][pos]["parameters"] = nlohmann::json::array();
        for (int p = 0; p < node->parameters.GetSize(); p++) {
          int idx = node->parameters.Get(p)->parameterIdx;
          double val = *(node->parameters.Get(p)->value);
          serialized["nodes"][pos]["parameters"][p] = {
            { "idx", idx },
            { "value", val }
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
    // output->inputs[0] = input;
    output->connectInput(input->outSockets.Get(0));
    int expectedIndex = 0;
    // create all the nodes and setup the parameters in the first pass
    for (auto sNode : serialized["nodes"]) {
      std::string className = sNode["type"];
      Node* node = NodeList::createNode(className);
      if (node == nullptr) { continue; }
      node->setup(sampleRate);
      node->X = sNode["position"][0];
      node->Y = sNode["position"][1];
      if (expectedIndex != sNode["idx"]) {
        WDBGMSG("Deserialization mismatched indexes, this will not load right\n");
      }
      nodes.Add(node);
      int paramIdx = 0;
      for (auto param : sNode["parameters"]) {
        if (paramIdx >= node->parameters.GetSize()) { break; }
        node->parameters.Get(paramIdx)->parameterIdx = param["idx"];
        *(node->parameters.Get(paramIdx)->value) = param["value"];
        paramIdx++;
      }
      paramManager->claimNode(node);
      if (graphics != nullptr && graphics->WindowIsOpen()) {
        node->setupUi(graphics);
      }
      expectedIndex++;
    }

    // link them all up accordingly in the second pass
    int currentNodeIdx = 0;
    for (auto sNode : serialized["nodes"]) {
      int currentInputIdx = 0;
      for (auto connection : sNode["inputs"]) {
        int inNodeIdx = connection[0];
        int inBufferIdx = connection[1];
        if (inNodeIdx >= 0 && nodes.Get(inNodeIdx) != nullptr) {
          //nodes.Get(currentInputIdx)->inputs[currentInputIdx] = nodes.Get(inNodeIdx);
          nodes.Get(currentNodeIdx)->connectInput(
            nodes.Get(inNodeIdx)->outSockets.Get(inBufferIdx),
            currentInputIdx
          );
        }
        else if (inNodeIdx == -1) {
          // thie index is -1 if the node is connected to the global input
          // if it's -2 it's not connected at all and we'll just leave it at a nullptr
          nodes.Get(currentNodeIdx)->connectInput(
            input->outSockets.Get(0),
            currentInputIdx
          );
        }
        currentInputIdx++;
      }
      currentNodeIdx++;
    }

    // connect the output nodes to the global output
    int outNodeIndex = serialized["output"][0];
    int outConnectionIndex = serialized["output"][1];
    if (nodes.Get(outNodeIndex) != nullptr) {
      output->connectInput(nodes.Get(outNodeIndex)->outSockets.Get(outConnectionIndex));
    }
  }
}

