#pragma once
#include <functional>
#include <string>

namespace guitard {
  class Node;
  namespace NodeList {
    struct NodeInfo;

    typedef std::function<Node* (NodeInfo*)> NodeConstructor;

    struct NodeInfo {
      String name; // Will be used internally for serialization, construction and so on
      String displayName; // only used to display
      String image;
      String categoryName;
      String description;
      NodeConstructor constructor;
    };
  }
}
