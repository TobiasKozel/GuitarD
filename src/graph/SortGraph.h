#pragma once
#include "src/node/Node.h"

namespace guitard {
  namespace SortGraph {
    /**
     * Does some sorting on the mNodes list so the graph can be computed with fewer attempts
     * Does not touch the positions of the nodes
     */
    inline void sortGraph(PointerList<Node>* nodes, Node* inNode, Node* outNode) {
      PointerList<Node> sorted;
      for (int i = 0; i < MAX_SOCKET_CONNECTIONS; i++) {
        // Put in the nodes which directly follow the input node
        NodeSocket* con = inNode->shared.socketsOut[0]->mConnectedTo[i];
        if (con != nullptr) {
          sorted.add(con->mParentNode);
        }
      }

      // Arbitrary depth
      for (int tries = 0; tries < 100; tries++) {
        for (int i = 0; i < sorted.size(); i++) {
          Node* node = sorted[i];
          for (int out = 0; out < node->shared.outputCount; out++) {
            NodeSocket* outSocket = node->shared.socketsOut[out];
            if (outSocket == nullptr) { continue; }
            for (int next = 0; next < MAX_SOCKET_CONNECTIONS; next++) {
              NodeSocket* nextSocket = outSocket->mConnectedTo[next];
              if (nextSocket == nullptr) { continue; }
              Node* nextNode = nextSocket->mParentNode;
              // Don't want to add duplicates or the output node
              if (sorted.find(nextNode) != -1) { continue; }
              sorted.add(nextNode);
            }
          }
        }
      }

      // Add in all the nodes which might not be connected or were missed because of the depth limit
      for (int i = 0; i < nodes->size(); i++) {
        Node* nextNode = nodes->get(i);
        if (sorted.find(nextNode) != -1) { continue; }
        sorted.add(nextNode);
      }

      nodes->clear();
      for (int i = 0; i < sorted.size(); i++) {
        Node* n = sorted[i];
        if (n == outNode) { continue; }
        nodes->add(n);
      }
    }
  }
}