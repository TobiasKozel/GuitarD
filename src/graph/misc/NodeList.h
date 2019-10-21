#pragma once
#include <map>
#include <functional>
#include "src/graph/misc/NodeInfo.h"

class Node;

namespace NodeList {
  typedef std::map<std::string, NodeInfo> NodeMap;

  NodeMap nodelist;

  Node* createNode(std::string name) {
    if (nodelist.find(name) != nodelist.end()) {
      return nodelist.at(name).constructor();
    }
    return nullptr;
  }

  void registerNode(NodeInfo info) {
    nodelist.insert(std::pair<std::string, NodeInfo>(info.name, info));
  }
};
