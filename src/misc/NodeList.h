#pragma once
#include <map>
#include <functional>
#include "src/node/NodeInfo.h"

class Node;

namespace NodeList {
  typedef std::map<std::string, NodeInfo> NodeMap;

  NodeMap nodelist;

  inline Node* createNode(const std::string name) {
    if (nodelist.find(name) != nodelist.end()) {
      return nodelist.at(name).constructor();
    }
    return nullptr;
  }

  inline NodeInfo* getInfo(const std::string name) {
    if (nodelist.find(name) != nodelist.end()) {
      return &(nodelist.find(name)->second);
    }
    return nullptr;
  }

  inline void registerNode(NodeInfo info) {
    if (nodelist.find(info.name) == nodelist.end()) {
      nodelist.insert(std::pair<std::string, NodeInfo>(info.name, info));
    }
  }
};
