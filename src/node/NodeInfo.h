#pragma once
#include <functional>
#include <string>

namespace guitard {
  class Node;
  namespace NodeList {
    struct NodeInfo;

    typedef std::function<Node* (NodeInfo*)> NodeConstructor;
    typedef std::function<NodeUi* (Node*)> NodeUiConstructor;

    struct NodeInfo {
      String name; // Will be used internally for serialization, construction and so on
      String displayName; // only used to display
      String categoryName;
      String description;
      String image;
      bool hidden = false;
      NodeConstructor constructor;
    };

    struct NodeUiInfo {
      String name; // The name of the node, not the UI class
      NodeUiConstructor constructor;
    };
  }
}
