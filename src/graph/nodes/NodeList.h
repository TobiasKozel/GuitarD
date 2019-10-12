#pragma once
#include "src/graph/nodes/stereo_tool/StereoToolNode.h"
#include "src/graph/nodes/simple_delay/SimpleDelayNode.h"
#include "src/graph/nodes/simple_cab/SimpleCabNode.h"

// TODO This file should be auto generated and also provide some way of getting a preview image for the gallery
Node* createNode(std::string name) {
  if (name == "StereoToolNode") {
    return new StereoToolNode();
  }
  if (name == "SimpleDelayNode") {
    return new SimpleDelayNode();
  }
  if (name == "SimpleCabNode") {
    return new SimpleCabNode();
  }
  return nullptr;
}