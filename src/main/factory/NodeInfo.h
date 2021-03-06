#pragma once
#include <functional>
#include <string>
#include "../../types/GTypes.h"

namespace guitard {
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
      bool hidden = false; // Hides the node in the sidebar, so only loading a preset can construct it
      NodeConstructor constructor;
    };

    struct NodeUiInfo {
      String name; // The name of the node, not the UI class
      NodeUiConstructor constructor;
    };
  }
}
