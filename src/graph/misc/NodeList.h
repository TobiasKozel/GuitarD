#pragma once
#include <map>
#include <functional>

class Node;

namespace NodeList {
  typedef std::function<Node* ()> NodeConstructor;

  struct NodeInfo {
    NodeConstructor constructor;
    std::string name;
    std::string dislayName;
    std::string image;
    std::string categoryName;
  };

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
