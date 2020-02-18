#pragma once
#include "../misc/constants.h"
#include "../types/types.h"

namespace guitard {
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
    sample** mParentBuffer = nullptr;
    std::function<void(bool)> callback;
    virtual void disconnect(NodeSocket* to = nullptr, bool other = true) = 0;
    virtual void disconnectAll() = 0;
    virtual void connect(NodeSocket* to, bool other = true) = 0;

    void lock(const bool other) const {
      if (callback && other) callback(true);
    }

    void unlock(const bool other) const {
      if (callback && other) callback(false);
    }

    Node* getConnectedNode(const int index = 0) const {
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

    virtual ~NodeSocket() { }
  };

  /**
   * Only allowed to have one connection
   * Will only write in the first index of mConnectedTo
   */
  struct NodeSocketIn : public NodeSocket {
    NodeSocketIn(Node* parent, const int index) {
      mParentNode = parent;
      mIndex = index;
      mIsInput = true;
    }
    /**
     * Disconnects any socket if "to" is a nullptr, checks and only disconnects if the sockets match otherwise
     */
    void disconnect(NodeSocket* to, const bool other) override {
      lock(other);
      if (mConnectedTo[0] != nullptr && (to == mConnectedTo[0] || to == nullptr)) {
        if (other) {
          mConnectedTo[0]->disconnect(this, false);
        }
        mConnected = false;
        mConnectedTo[0] = nullptr;
      }
      unlock(other);
    }

    void disconnectAll() override {
      disconnect(mConnectedTo[0], true);
    }

    void connect(NodeSocket* to, const bool other) override {
      if (to->mIsInput == mIsInput) { return; }
      if (to->mParentNode == mParentNode) { return; }
      /**
       * Input sockets can only hold one connection, so disconnect it in case
       * there was already a socket connected
       */
      disconnect(nullptr, true);
      lock(other);
      if (other) {
        to->connect(this, false);
      }
      mConnected = true;
      mConnectedTo[0] = to;
      unlock(other);
    }
  };

  struct NodeSocketOut : public NodeSocket {
    NodeSocketOut(Node* parent, int index) {
      mParentNode = parent;
      mIndex = index;
      mIsInput = false;
    }

    void updatedConnected() {
      int sortedIndex = 0;
      mConnected = false;
      for (int i = 0; i < MAX_SOCKET_CONNECTIONS; i++) {
        if (mConnectedTo[i] != nullptr) {
          mConnected = true;
          mConnectedTo[sortedIndex] = mConnectedTo[i];
          if (sortedIndex != i) {
            mConnectedTo[i] = nullptr;
          }
          sortedIndex++;
        }
      }
    }

    void disconnect(NodeSocket* to, const bool other) override {
      lock(other);
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
      unlock(other);
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

    void connect(NodeSocket* to, const bool other) override {
      if (to->mIsInput == mIsInput) { return; }
      NodeSocketIn* toIn = dynamic_cast<NodeSocketIn*>(to);
      if (toIn == nullptr) { return; }
      if (toIn->mParentNode == mParentNode) { return; }
      for (int i = 0; i < MAX_SOCKET_CONNECTIONS; i++) {
        if (mConnectedTo[i] == to) {
          // assert(false);
          return;
        }
      }
      lock(other);
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
          assert(false);
        }
      }
      updatedConnected();
      unlock(other);
    }
  };
}