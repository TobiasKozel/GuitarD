#pragma once
#include <functional>
#include <string>

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
}
