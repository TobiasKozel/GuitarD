#pragma once
#include "IPlugConstants.h"

class Node;

class NodeSocket {
public:
  // Constructor if it's a input socket
  NodeSocket(int pIndex, Node* pNode = nullptr) {
    isInput = true;
    ownIndex = pIndex;
    buffer = nullptr;
    connectedNode = pNode;
    connectedBufferIndex = -1;
    X = Y = 0;
  }

  /**
   * Constructor if it's a output socket, will need to know
   * the buffer
   */
  NodeSocket(int pIndex, Node* pNode, iplug::sample** pBuffer) {
    isInput = false;
    ownIndex = pIndex;
    buffer = pBuffer;
    connectedNode = pNode;
    connectedBufferIndex = -1;
    X = Y = 0;
  }

  Node* connectedNode;
  int connectedBufferIndex;
  int ownIndex;
  iplug::sample** buffer;
  bool isInput;
  float X;
  float Y;
};