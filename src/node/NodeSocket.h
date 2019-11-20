#pragma once
#include "IPlugConstants.h"
#include "src/misc/MessageBus.h"

class Node;

struct NodeSocket {
  bool mIsInput = false;
  NodeSocket* mConnectedTo = nullptr;
  Node* mParentNode = nullptr;
  int mIndex = -1;
  iplug::sample** mParentBuffer = nullptr;
  float mX = 0;
  float mY = 0;

  Node* mConnectedNode = nullptr;
  int mConnectedSocketIndex = -1;
  // TODOG Get the message bus ouf here, since it shouldn't be needed without the GUI running
  MessageBus::Bus* mBus = nullptr;

  /**
   * Constructor if it's a input socket, meaning it has no buffer
   * (Nodes only have an output buffer)
   */
  NodeSocket(MessageBus::Bus* pBus, const int pIndex, Node* pNode) {
    mBus = pBus;
    mIsInput = true;
    mParentNode = pNode;
    mIndex = pIndex;
  }

  /**
   * Constructor if it's a output socket, will need to know
   * the buffer since other sockets will read from it
   */
  NodeSocket(MessageBus::Bus* pBus, int pIndex, Node* pNode, iplug::sample** pBuffer) {
    mBus = pBus;
    mIsInput = false;
    mParentNode = pNode;
    mIndex = pIndex;
    mParentBuffer = pBuffer;
  }

  ~NodeSocket() {
    disconnect();
  }

  void disconnect() {
    MessageBus::fireEvent<bool>(mBus, MessageBus::AwaitAudioMutex, false);
    if (mIsInput) {
      mConnectedNode = nullptr;
      mConnectedSocketIndex = -1;
      mConnectedTo = nullptr;
    }
    else {
      MessageBus::fireEvent<NodeSocket*>(mBus, MessageBus::DisconnectSocket, this);
    }
  }

  void connect(NodeSocket* to) {
    MessageBus::fireEvent<bool>(mBus, MessageBus::AwaitAudioMutex, false);
    if (to->mIsInput == mIsInput) {
      WDBGMSG("Trying to connect an input to input / output to output!");
      return;
    }
    if (mIsInput) {
      mConnectedTo = to;
      mConnectedNode = to->mParentNode;
      mConnectedSocketIndex = to->mIndex;
    }
    else {
      to->mConnectedTo = this;
      to->mConnectedNode = mParentNode;
      to->mConnectedSocketIndex = mIndex;
    }

  }
};
