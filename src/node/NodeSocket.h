#pragma once
#include "IPlugConstants.h"
#include "src/misc/MessageBus.h"

class Node;

class NodeSocket {
public:
  /**
   * Constructor if it's a input socket, meaning it has no buffer
   * (Nodes only have an output buffer)
   */

  NodeSocket(MessageBus::Bus* pBus, int pIndex, Node* pNode) {
    mBus = pBus;
    isInput = true;
    connectedTo = nullptr;
    parentNode = pNode;
    index = pIndex;
    parentBuffer = nullptr;
    X = Y = 0;
    connectedSocketIndex = -1;
    connectedNode = nullptr;
  }

  /**
   * Constructor if it's a output socket, will need to know
   * the buffer since other sockets will read from it
   */
  NodeSocket(MessageBus::Bus* pBus, int pIndex, Node* pNode, iplug::sample** pBuffer) {
    mBus = pBus;
    isInput = false;
    connectedTo = nullptr;
    parentNode = pNode;
    index = pIndex;
    parentBuffer = pBuffer;
    X = Y = 0;
    connectedSocketIndex = -1;
    connectedNode = nullptr;
  }

  ~NodeSocket() {
    disconnect();
  }

  void disconnect() {
    MessageBus::fireEvent<bool>(mBus, MessageBus::AwaitAudioMutex, false);
    if (isInput) {
      connectedNode = nullptr;
      connectedSocketIndex = -1;
      connectedTo = nullptr;
    }
    else {
      MessageBus::fireEvent<NodeSocket*>(mBus, MessageBus::DisconnectSocket, this);
    }
  }

  void connect(NodeSocket* to) {
    MessageBus::fireEvent<bool>(mBus, MessageBus::AwaitAudioMutex, false);
    if (to->isInput == isInput) {
      WDBGMSG("Trying to connect an input to input / output to output!");
      return;
    }
    if (isInput) {
      connectedTo = to;
      connectedNode = to->parentNode;
      connectedSocketIndex = to->index;
    }
    else {
      to->connectedTo = this;
      to->connectedNode = parentNode;
      to->connectedSocketIndex = index;
    }

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
  MessageBus::Bus* mBus;
};