#pragma once
#include "src/misc/constants.h"
class Node;

/**
 * Base class of a node socket, only the implementations of disconnect and connect differ
 * The member variables are identical
 */
struct NodeSocket {
  bool mIsInput = false;
  float mX = 0;
  float mY = 0;
  Node* mParentNode = nullptr;
  int mIndex = -1;
  bool mConnected = false;
  NodeSocket* mConnectedTo[MAX_SOCKET_CONNECTIONS] = { nullptr };
  // Buffer is only relevant for a output node
  iplug::sample** mParentBuffer = nullptr;

  virtual void disconnect(NodeSocket* to = nullptr, bool other = true) = 0;
  virtual void disconnectAll() = 0;
  virtual void connect(NodeSocket* to, bool other = true) = 0;

  Node* getConnectedNode(int index = 0) const {
    if (mConnectedTo[index] != nullptr) {
      return mConnectedTo[index]->mParentNode;
    }
    return nullptr;
  }

  int getConnectedSocketIndex(int index = 0) {
    if (mConnectedTo[index] != nullptr) {
      return mConnectedTo[index]->mIndex;
    }
    return -1;
  }
};

/**
 * Only allowed to have one connection
 * Will only write in the first index of mConnectedTo
 */
struct NodeSocketIn : public NodeSocket {
  NodeSocketIn(Node* parent, int index) {
    mParentNode = parent;
    mIndex = index;
    mIsInput = true;
  }
  /**
   * Disconnects any socket if to is a nullptr, checks and only disconnects if the sockets match otherwise
   */
  void disconnect(NodeSocket* to, bool other) override {
    if (mConnectedTo[0] != nullptr && (to == mConnectedTo[0] || to == nullptr)) {
      if (other) {
        mConnectedTo[0]->disconnect(this, false);
      }
      mConnected = false;
      mConnectedTo[0] = nullptr;
    }
  }

  void disconnectAll() override {
    disconnect(mConnectedTo[0], true);
    mConnected = false;
  }

  void connect(NodeSocket* to, bool other) override {
    if (to->mIsInput == mIsInput) { return; }
    if (other) {
      disconnect(to, false);
      to->connect(this, false);
    }
    mConnected = true;
    mConnectedTo[0] = to;
  }
};

struct NodeSocketOut : public NodeSocket {
  NodeSocketOut(Node* parent, int index, iplug::sample** buffer) {
    mParentNode = parent;
    mIndex = index;
    mParentBuffer = buffer;
    mIsInput = false;
  }

  void updatedConnected() {
    for (int i = 0; i < MAX_SOCKET_CONNECTIONS; i++) {
      if (mConnectedTo[i] != nullptr) {
        mConnected = true;
        return;
      }
    }
    mConnected = false;
  }

  void disconnect(NodeSocket* to, bool other) override {
    for (int i = 0; i < MAX_SOCKET_CONNECTIONS; i++) {
      if (mConnectedTo[i] != nullptr && mConnectedTo[i] == to) {
        mConnectedTo[i] = nullptr;
        if (other) {
          to->disconnect(nullptr, false);
        }
        break;
      }
    }
    updatedConnected();
  }

  void disconnectAll() override {
    for (int i = 0; i < MAX_SOCKET_CONNECTIONS; i++) {
      if (mConnectedTo[i] != nullptr) {
        mConnectedTo[i]->disconnect(this, false);
        mConnectedTo[i] = nullptr;
      }
    }
    mConnected = false;
  }

  void connect(NodeSocket* to, bool other) override {
    if (to->mIsInput == mIsInput) { return; }
    NodeSocketIn* toIn = dynamic_cast<NodeSocketIn*>(to);
    if (toIn != nullptr) {
      for (int i = 0; i < MAX_SOCKET_CONNECTIONS; i++) {
        if (mConnectedTo[i] == to) {
          assert(true);
        }
      }
      for (int i = 0; i < MAX_SOCKET_CONNECTIONS; i++) {
        if (mConnectedTo[i] == nullptr) {
          mConnectedTo[i] = toIn;
          if (toIn->mConnectedTo[0] != this) {
            if (other) {
              toIn->connect(this, false);
            }
          }
          break;
        }
        if (i == MAX_SOCKET_CONNECTIONS - 1) {
          // Couldn't find a free slot
          assert(true);
        }
      }
      updatedConnected();
    }
  }
};
