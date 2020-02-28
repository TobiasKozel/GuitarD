#pragma once
#include <map>
#include <functional>
#include "../node/NodeInfo.h"

namespace guitard {
  class Node;

  namespace NodeList {
    typedef std::map<String, NodeInfo> NodeMap;
    typedef std::map<String, NodeUiInfo> NodeUiMap;
    /**
     * This is a global list of all nodes shared across all instances
     * of the plugin in the process
     */
    NodeMap nodeList;
    NodeUiMap nodeUiList;

    /**
     * All nodes except input and output will be constructed here
     */
    inline Node* createNode(const String& name) {
      if (nodeList.find(name) != nodeList.end()) {
        NodeInfo& info = nodeList.at(name);
        Node* n = info.constructor(&info);
        // WDBGMSG(n->getLicense().c_str());
        return n;
      }
      return nullptr;
    }

    inline NodeUi* createNodeUi(Node* node) {
      String name = node->shared.info->name;
      if (nodeUiList.find(name) != nodeUiList.end()) {
        return nodeUiList.at(name).constructor(node);
      }

      // Check if there's a Node with the name and create a generic UI
      if (nodeList.find(name) != nodeList.end()) {

      }
      return nullptr; // There's no Node to create the ui for
    }

    inline NodeInfo* getInfo(const String& name) {
      if (nodeList.find(name) != nodeList.end()) {
        return &(nodeList.find(name)->second);
      }
      return nullptr;
    }

    inline void registerNode(NodeInfo info) {
      if (nodeList.find(info.name) == nodeList.end()) {
        nodeList.insert(std::pair<String, NodeInfo>(info.name, info));
      }
    }

    inline void registerNodeUi(NodeUiInfo info) {
      if (nodeUiList.find(info.name) == nodeUiList.end()) {
        nodeUiList.insert(std::pair<String, NodeUiInfo>(info.name, info));
      }
    }

    template <class T>
    struct RegisterProxy {
      /**
       * Will be called when a node is registered
       */
      RegisterProxy(NodeInfo pInfo) {
        if (pInfo.constructor == nullptr) {
          pInfo.constructor = [](NodeInfo* info) {
            return new T(info);
          };
        }
        registerNode(pInfo);
      }

      /**
       * Will be called when the UI to a node is registered
       */
      RegisterProxy(NodeUiInfo pInfo) {
        pInfo.constructor = [](Node* node) {
          return new T(node);
        };
        registerNodeUi(pInfo);
      }
    };
  }
}


/**
 * Below lies some macro magic to define global variables of the RegisterProxy type
 * inside of the constructor of the class the registerNode function will be called
 * and the node registered. Not sure if I like this, but it works
 */
#define GUITARD_STRX(a) str(a)
#define GUITARD_STR(a) #a

/**
 * The first argument has to be the node class itself
 * the following arguments will be displayName,categoryName, description, image and hidden
 * you can supply a custom constructor after that, but no idea why you would need that
 */
#define GUITARD_REGISTER_NODE(type, ...) \
namespace NodeList { \
  RegisterProxy<type> register##type(NodeInfo{ GUITARD_STR(type), __VA_ARGS__ });\
}\

/**
 * First argument is the class name of the node the UI is for
 * second one is the classname of the ui
 */
#define GUITARD_REGISTER_NODE_UI(type, ui) \
namespace NodeList { \
  RegisterProxy<ui> register##ui(NodeUiInfo{ GUITARD_STR(type) });\
}\
