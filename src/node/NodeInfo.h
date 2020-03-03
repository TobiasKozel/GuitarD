#pragma once
#include <functional>
#include <string>

namespace guitard {
  class Node;
  class NodeUi;
  namespace MessageBus {
    struct Bus;
  }
  namespace NodeList {
    struct NodeInfo;

    typedef std::function<Node* (NodeInfo*)> NodeConstructor;
    typedef std::function<NodeUi* (Node*, MessageBus::Bus*)> NodeUiConstructor;

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
