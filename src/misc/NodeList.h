#pragma once
#include <map>
#include <functional>
#include "src/node/NodeInfo.h"

class Node;

namespace NodeList {
  typedef std::map<std::string, NodeInfo> NodeMap;

  NodeMap nodelist;

  /**
   * All nodes except input and output will be constructed here
   */
  inline Node* createNode(const std::string name) {
    if (nodelist.find(name) != nodelist.end()) {
      Node* n = nodelist.at(name).constructor();
      WDBGMSG(n->getLicense().c_str());
      return n;
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
