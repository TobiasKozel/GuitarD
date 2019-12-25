#pragma once
#include "src/node/Node.h"

namespace FormatGraph {
  /**
 * Recursively resets all the positions of nodes to (0, 0)
 */
  static void resetBranchPos(Node* node) {
    if (node == nullptr || node->shared.type == "FeedbackNode") { return; }
    node->mUi->setTranslation(0, 0);
    NodeSocket* socket = nullptr;
    for (int i = 0; i < node->shared.outputCount; i++) {
      socket = node->shared.socketsOut[i];
      for (int j = 0; j < MAX_SOCKET_CONNECTIONS; j++) {
        if (socket->mConnectedTo[j] != nullptr) {
          if (socket->mConnectedTo[j]->mIndex == 0) {
            resetBranchPos(socket->mConnectedTo[j]->mParentNode);
          }
        }
      }
    }
  }

  /**
   * Recursively sorts nodes
   */
  Coord2D arrangeBranch(Node* node, Coord2D pos) {
    if (node == nullptr || node->shared.type == "FeedbackNode") {
      return pos;
    }
    const float halfWidth = node->shared.width * 0.5;
    const float halfHeight = node->shared.height * 0.5;
    const float padding = 50;
    pos.x += halfWidth + padding;
    node->mUi->setTranslation(pos.x, pos.y);
    pos.x += halfWidth + padding;
    float nextX = 0;
    NodeSocket* socket = nullptr;
    for (int i = 0; i < node->shared.outputCount; i++) {
      socket = node->shared.socketsOut[i];
      for (int j = 0; j < MAX_SOCKET_CONNECTIONS; j++) {
        if (socket->mConnectedTo[j] != nullptr) {
          if (socket->mConnectedTo[j]->mIndex == 0) {
            Coord2D branch = arrangeBranch(socket->mConnectedTo[j]->mParentNode, pos);
            pos.y += node->shared.height + padding;
            if (pos.y < branch.y) {
              pos.y = branch.y;
            }
            if (branch.x > nextX) {
              nextX = branch.x;
            }
          }
        }
      }
    }
    return Coord2D{ nextX, pos.y };
  }
}
