#pragma once
#include "thirdparty/json.hpp"
#include "src/node/Node.h"
#include "src/misc/NodeList.h"
#include "src/parameter/ParameterManager.h"


namespace Serializer {

  enum SpecialNode {
    NoNode = -2,
    InputNode = -1
  };

  inline void serialize(nlohmann::json& serialized, WDL_PtrList<Node>& nodes, Node* input, Node* output) {
    serialized["input"]["gain"] = 1.0;
    serialized["input"]["position"] = {
      input->mX, input->mY
    };
    serialized["nodes"] = nlohmann::json::array();
    for (int i = 0; i < nodes.GetSize(); i++) {
      Node* node = nodes.Get(i);
      serialized["nodes"][i]["position"] = { node->mX, node->mY };
      // The index shouldn't really matter since they're all in order
      serialized["nodes"][i]["idx"] = i;
      serialized["nodes"][i]["type"] = node->mType;
      serialized["nodes"][i]["inputs"] = nlohmann::json::array();
      for (int prev = 0; prev < node->mInputCount; prev++) {
        Node* cNode = node->mSocketsIn.Get(prev)->mConnectedNode;
        if (cNode == nullptr) {
          serialized["nodes"][i]["inputs"][prev] = { NoNode, 0 };
        }
        else if (cNode == input) {
          serialized["nodes"][i]["inputs"][prev] = { InputNode, 0 };
        }
        else {
          serialized["nodes"][i]["inputs"][prev] = {
            nodes.Find(cNode),
            node->mSocketsIn.Get(prev)->mConnectedSocketIndex
          };
        }
      }
      serialized["nodes"][i]["parameters"] = nlohmann::json::array();
      for (int p = 0; p < node->mParameters.GetSize(); p++) {
        ParameterCoupling* para = node->mParameters.Get(p);
        const char* name = para->name;
        double val = para->parameter != nullptr ? para->parameter->Value() : para->baseValue;
        int idx = para->parameterIdx;
        serialized["nodes"][i]["parameters"][p] = {
          { "name", name },
          { "idx", idx},
          { "value", val }
        };
      }
    }
    // Handle the output node
    serialized["output"]["gain"] = 1.0;
    serialized["output"]["position"] = {
      output->mX, output->mY
    };
    Node* lastNode = output->mSocketsIn.Get(0)->mConnectedNode;
    int lastNodeIndex = NoNode;
    if (lastNode == input) {
      lastNodeIndex = InputNode;
    }
    else if (lastNode != nullptr) {
      lastNodeIndex = nodes.Find(lastNode);
    }
    serialized["output"]["inputs"][0] = {
      lastNodeIndex,
      output->mSocketsIn.Get(0)->mConnectedSocketIndex
    };
  }

  inline void deserialize(
    nlohmann::json& serialized, WDL_PtrList<Node>& nodes, Node* output, Node* input, int sampleRate,
    ParameterManager* paramManager, MessageBus::Bus* pBus
  ) {
    
    output->connectInput(nullptr, 0);

    if (input->mUi != nullptr) {
      input->mUi->setTranslation(
        input->mX = serialized["input"]["position"][0],
        input->mY = serialized["input"]["position"][1]
      );
    }
    else {
      input->mX = serialized["input"]["position"][0];
      input->mY = serialized["input"]["position"][1];
    }

    int expectedIndex = 0;

    // create all the nodes and setup the parameters in the first pass
    for (auto sNode : serialized["nodes"]) {
      const std::string className = sNode["type"];
      Node* node = NodeList::createNode(className);
      if (node == nullptr) { continue; }
      node->mX = sNode["position"][0];
      node->mY = sNode["position"][1];
      node->setup(pBus, sampleRate);
      if (expectedIndex != sNode["idx"]) {
        WDBGMSG("Deserialization mismatched indexes, this will not load right\n");
      }
      nodes.Add(node);
      for (auto param : sNode["parameters"]) {
        string name = param["name"];
        for (int i = 0; i < node->mParameters.GetSize(); i++) {
          ParameterCoupling* para = node->mParameters.Get(i);
          if (para->name == name) {
            para->parameterIdx = param["idx"];
            *(para->value) = param["value"];
          }
        }
      }
      paramManager->claimNode(node);
      expectedIndex++;
    }

    // link them all up accordingly in the second pass
    int currentNodeIdx = 0;
    for (auto sNode : serialized["nodes"]) {
      int currentInputIdx = 0;
      for (auto connection : sNode["inputs"]) {
        const int inNodeIdx = connection[0];
        const int inBufferIdx = connection[1];
        if (inNodeIdx >= 0 && nodes.Get(inNodeIdx) != nullptr) {
          nodes.Get(currentNodeIdx)->connectInput(
            nodes.Get(inNodeIdx)->mSocketsOut.Get(inBufferIdx),
            currentInputIdx
          );
        }
        else if (inNodeIdx == InputNode) {
          // if it's NoNode it's not connected at all and we'll just leave it at a nullptr
          nodes.Get(currentNodeIdx)->connectInput(
            input->mSocketsOut.Get(0),
            currentInputIdx
          );
        }
        currentInputIdx++;
      }
      currentNodeIdx++;
    }

    // connect the output nodes to the global output
    const int outNodeIndex = serialized["output"]["inputs"][0][0];
    const int outConnectionIndex = serialized["output"]["inputs"][0][1];
    if (nodes.Get(outNodeIndex) != nullptr) {
      output->connectInput(
        nodes.Get(outNodeIndex)->mSocketsOut.Get(outConnectionIndex)
      );
    }
    else if (outNodeIndex == InputNode) {
      output->connectInput(input->mSocketsOut.Get(0));
    }

    //output->X = serialized["output"]["position"][0];
    //output->Y = serialized["output"]["position"][1];
    if (output->mUi != nullptr) {
      output->mUi->setTranslation(
        serialized["output"]["position"][0],
        serialized["output"]["position"][1]
      );
    }
    else {
      output->mX = serialized["output"]["position"][0];
      output->mY = serialized["output"]["position"][1];
    }
  }
}

