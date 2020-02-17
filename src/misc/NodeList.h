#pragma once
#include <map>
#include <functional>
#include "../node/NodeInfo.h"

namespace guitard {
  class Node;

  namespace NodeList {
    typedef std::map<String, NodeInfo> NodeMap;

    NodeMap nodelist; // This is a global variable!

    /**
     * All nodes except input and output will be constructed here
     */
    inline Node* createNode(const String& name) {
      if (nodelist.find(name) != nodelist.end()) {
        NodeInfo& info = nodelist.at(name);
        Node* n = info.constructor(&info);
        WDBGMSG(n->getLicense().c_str());
        return n;
      }
      return nullptr;
    }

    inline NodeInfo* getInfo(const String& name) {
      if (nodelist.find(name) != nodelist.end()) {
        return &(nodelist.find(name)->second);
      }
      return nullptr;
    }

    inline void registerNode(NodeInfo info) {
      if (nodelist.find(info.name) == nodelist.end()) {
        nodelist.insert(std::pair<String, NodeInfo>(info.name, info));
      }
    }
  };
}