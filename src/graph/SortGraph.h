#pragma once
#include "src/node/Node.h"

namespace guitard {
  namespace SortGraph {
    /**
     * Does some sorting on the mNodes list so the graph can be computed with fewer attempts
     * Does not touch the positions of the nodes
     */
    inline void sortGraph(WDL_PtrList<Node>& nodes, Node* inNode, Node* outNode, WDL_Mutex* isProcessing = nullptr) {
      WDL_PtrList<Node> sorted;
      for (int i = 0; i < MAX_SOCKET_CONNECTIONS; i++) {
        // Put in the nodes which directly follow the input node
        NodeSocket* con = inNode->shared.socketsOut[0]->mConnectedTo[i];
        if (con != nullptr) {
          sorted.Add(con->mParentNode);
        }
      }

      // Arbitrary depth
      for (int tries = 0; tries < 100; tries++) {
        for (int i = 0; i < sorted.GetSize(); i++) {
          Node* node = sorted.Get(i);
          for (int out = 0; out < node->shared.outputCount; out++) {
            NodeSocket* outSocket = node->shared.socketsOut[out];
            if (outSocket == nullptr) { continue; }
            for (int next = 0; next < MAX_SOCKET_CONNECTIONS; next++) {
              NodeSocket* nextSocket = outSocket->mConnectedTo[next];
              if (nextSocket == nullptr) { continue; }
              Node* nextNode = nextSocket->mParentNode;
              // Don't want to add duplicates or the output node
              if (sorted.Find(nextNode) != -1) { continue; }
              sorted.Add(nextNode);
            }
          }
        }
      }

      // Add in all the nodes which might not be connected or were missed because of the depth limit
      for (int i = 0; i < nodes.GetSize(); i++) {
        Node* nextNode = nodes.Get(i);
        if (sorted.Find(nextNode) != -1) { continue; }
        sorted.Add(nextNode);
      }

      WDL_MutexLock* mutex;
      if (isProcessing != nullptr) {
        mutex = new WDL_MutexLock(isProcessing);
      }
      nodes.Empty(false);
      for (int i = 0; i < sorted.GetSize(); i++) {
        Node* n = sorted.Get(i);
        if (n == outNode) { continue; }
        nodes.Add(n);
      }
      if (isProcessing != nullptr) {
        delete mutex;
      }
    }
  }
}