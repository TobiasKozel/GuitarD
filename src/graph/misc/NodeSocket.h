#pragma once
#include "IPlugConstants.h"

class Node;

class NodeSocket {
public:
  /**
   * Constructor if it's a input socket, meaning it has no buffer
   * (Nodes only have an output buffer)
   */

  NodeSocket(int pIndex, Node* pNode) {
    isInput = true;
    connectedTo = nullptr;
    parentNode = pNode;
    index = pIndex;
    parentBuffer = nullptr;
    X = Y = 0;
    connectedSocketIndex = -1;
  }

  /**
   * Constructor if it's a output socket, will need to know
   * the buffer since other sockets will read from it
   */
  NodeSocket(int pIndex, Node* pNode, iplug::sample** pBuffer) {
    isInput = false;
    connectedTo = nullptr;
    parentNode = pNode;
    index = pIndex;
    parentBuffer = pBuffer;
    X = Y = 0;
    connectedSocketIndex = -1;
  }

  ~NodeSocket() {
    disconnect();
  }

  void disconnect() {
    if (connectedTo != nullptr) {
      connectedTo->connectedTo = nullptr;
      connectedTo->connectedNode = nullptr;
      connectedTo->connectedSocketIndex = -1;

      connectedTo = nullptr;
      connectedNode = nullptr;
      connectedSocketIndex = -1;
    }
  }

  void connect(NodeSocket* to) {
    if (to->isInput == isInput) {
      WDBGMSG("Trying to connect an input to input / output to output!");
      return;
    }
    if (connectedTo != nullptr) {
      disconnect();
    }
    to->connectedTo = this;
    to->connectedNode = parentNode;
    to->connectedSocketIndex = index;

    connectedTo = to;
    connectedNode = to->parentNode;
    connectedSocketIndex = to->index;
  }

  bool isInput;
  NodeSocket* connectedTo;
  Node* parentNode;
  int index;
  iplug::sample** parentBuffer;
  float X;
  float Y;

  Node* connectedNode;
  int connectedSocketIndex;
};