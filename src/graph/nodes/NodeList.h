#pragma once
#include "src/graph/Node.h"
#include "src/graph/nodes/stereo_tool/StereoToolNode.h"
#include "src/graph/nodes/simple_delay/SimpleDelayNode.h"

Node* createNode(std::string name) {
  if (name == "StereoToolNode") {
    return new StereoToolNode();
  }
  if (name == "SimpleDelayNode") {
    return new SimpleDelayNode();
  }
  return nullptr;
}