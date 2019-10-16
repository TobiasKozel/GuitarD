#pragma once
#include "IPlugConstants.h"

class Node;

class NodeSocket {
public:
  // Constructor if it's a input socket
  NodeSocket(int pIndex) {
    isInput = true;
    ownIndex = pIndex;
    buffer = nullptr;
    connectedNode = nullptr;
    connectedBufferIndex = -1;
  }

  /**
   * Constructor if it's a output socket, will need to know
   * the buffer and which node it's attached to
   */
  NodeSocket(int pIndex, Node* pNode, iplug::sample** pBuffer = nullptr) {
    isInput = false;
    ownIndex = pIndex;
    buffer = pBuffer;
    connectedNode = pNode;
    connectedBufferIndex = -1;
  }
  Node* connectedNode;
  int connectedBufferIndex;
  int ownIndex;
  iplug::sample** buffer;
  bool isInput;
};