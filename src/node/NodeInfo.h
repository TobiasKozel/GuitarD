#pragma once
#include <functional>
#include <string>

namespace guitard {
  class Node;
  namespace NodeList {
    struct NodeInfo;

    typedef std::function<Node* (NodeInfo)> NodeConstructor;

    struct NodeInfo {
      std::string name; // Will used internally for serialization, construction and so on
      std::string displayName; // only used to display
      std::string image;
      std::string categoryName;
      NodeConstructor constructor;
    };
  }
}
