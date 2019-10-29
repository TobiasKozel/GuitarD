#pragma once
#include "thirdparty/json.hpp"
#include "IGraphics.h"
#include "src/node/Node.h"
#include "src/misc/NodeList.h"
#include "src/parameter/ParameterManager.h"


namespace serializer {

  enum SpecialNode {
    NoNode = -2,
    InputNode = -1
  };

  void serialize(nlohmann::json& serialized, WDL_PtrList<Node>& nodes, Node* input, Node* output) {
    serialized["input"]["gain"] = 1.0;
    serialized["input"]["position"] = {
      input->X, input->Y
    };
    serialized["nodes"] = nlohmann::json::array();
    for (int i = 0; i < nodes.GetSize(); i++) {
      Node* node = nodes.Get(i);
      serialized["nodes"][i]["position"] = { node->X, node->Y };
      // The index shouldn't really matter since they're all in order
      serialized["nodes"][i]["idx"] = i;
      serialized["nodes"][i]["type"] = node->type;
      serialized["nodes"][i]["inputs"] = nlohmann::json::array();
      for (int prev = 0; prev < node->inputCount; prev++) {
        Node* cNode = node->inSockets.Get(prev)->connectedNode;
        if (cNode == nullptr) {
          serialized["nodes"][i]["inputs"][prev] = { NoNode, 0 };
        }
        else if (cNode == input) {
          serialized["nodes"][i]["inputs"][prev] = { InputNode, 0 };
        }
        else {
          serialized["nodes"][i]["inputs"][prev] = {
            nodes.Find(cNode),
            node->inSockets.Get(prev)->connectedSocketIndex
          };
        }
      }
      serialized["nodes"][i]["parameters"] = nlohmann::json::array();
      for (int p = 0; p < node->parameters.GetSize(); p++) {
        int idx = node->parameters.Get(p)->parameterIdx;
        double val = *(node->parameters.Get(p)->value);
        serialized["nodes"][i]["parameters"][p] = {
          { "idx", idx },
          { "value", val }
        };
      }
    }
    // Handle the output node
    serialized["output"]["gain"] = 1.0;
    serialized["output"]["position"] = {
      output->X, output->Y
    };
    Node* lastNode = output->inSockets.Get(0)->connectedNode;
    int lastNodeIndex = NoNode;
    if (lastNode == input) {
      lastNodeIndex = InputNode;
    }
    else if (lastNode != nullptr) {
      lastNodeIndex = nodes.Find(lastNode);
    }
    serialized["output"]["inputs"][0] = {
      lastNodeIndex,
      output->inSockets.Get(0)->connectedSocketIndex
    };
  }

  void deserialize(
    nlohmann::json& serialized, WDL_PtrList<Node>& nodes, Node* output, Node* input, int sampleRate,
    ParameterManager* paramManager, iplug::igraphics::IGraphics* graphics
  ) {
    
    output->connectInput(nullptr, 0);
    input->mUi->setTranslation(
      serialized["input"]["position"][0],
      serialized["input"]["position"][1]
    );
    int expectedIndex = 0;

    // create all the nodes and setup the parameters in the first pass
    for (auto sNode : serialized["nodes"]) {
      std::string className = sNode["type"];
      Node* node = NodeList::createNode(className);
      if (node == nullptr) { continue; }
      node->X = sNode["position"][0];
      node->Y = sNode["position"][1];
      node->setup(sampleRate);
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
          nodes.Get(currentNodeIdx)->connectInput(
            nodes.Get(inNodeIdx)->outSockets.Get(inBufferIdx),
            currentInputIdx
          );
        }
        else if (inNodeIdx == InputNode) {
          // if it's NoNode it's not connected at all and we'll just leave it at a nullptr
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
    int outNodeIndex = serialized["output"]["inputs"][0][0];
    int outConnectionIndex = serialized["output"]["inputs"][0][1];
    if (nodes.Get(outNodeIndex) != nullptr) {
      output->connectInput(
        nodes.Get(outNodeIndex)->outSockets.Get(outConnectionIndex)
      );
    }
    else if (outNodeIndex == InputNode) {
      output->connectInput(input->outSockets.Get(0));
    }
    output->mUi->setTranslation(
      serialized["output"]["position"][0],
      output->Y = serialized["output"]["position"][1]
    );
  }
}

