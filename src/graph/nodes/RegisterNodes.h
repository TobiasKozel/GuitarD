#pragma once
#include "src/graph/misc/NodeList.h"
#include "src/graph/nodes/stereo_tool/StereoToolNode.h"
#include "src/graph/nodes/simple_drive/SimpleDriveNode.h"
#include "src/graph/nodes/simple_delay/SimpleDelayNode.h"
#include "src/graph/nodes/simple_cab/SimpleCabNode.h"
#include "src/graph/nodes/crybaby/CryBabyNode.h"


namespace NodeList {
  void registerNodes() {
    NodeList::registerNode(NodeList::NodeInfo {
      []() { return new StereoToolNode(); },
      "StereoToolNode",
      "Stereo Tool",
      "asd",
      "Tools"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new SimpleDriveNode(); },
      "SimpleDriveNode",
      "Simple Drive",
      "asd",
      "Distortion"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new SimpleDelayNode(); },
      "SimpleDelayNode",
      "Mono Delay",
      "asd",
      "Delays/Reverbs"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new SimpleCabNode(); },
      "SimpleCabNode",
      "Simple Cabinet",
      "asd",
      "Cabinets"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new CryBabyNode(); },
      "CryBabyNode",
      "Crybaby",
      "asd",
      "Filters"
    });
  }
}