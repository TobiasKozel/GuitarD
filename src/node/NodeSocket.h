#pragma once
#include "../misc/constants.h"
#include "../types/types.h"
#include "../types/gstructs.h"

namespace guitard {
  sample MONO_EMPTY_BUFFER[GUITARD_MAX_BUFFER] = { 0.0 };
  /**
   * This is an empty buffer to be used as a dummy for unconnected dsp
   */
  sample* EMPTY_BUFFER[2] = { MONO_EMPTY_BUFFER, MONO_EMPTY_BUFFER };
  class Node;
  /**
   * Base class of a node socket, only the implementations of disconnect and connect differ
   * The member variables are identical
   */
  struct NodeSocket {
    // Will not change over the lifetime of the object
    bool mIsInput;
    Node* mParentNode = nullptr; // The node it belongs to
    int mIndex; // The index of the input or output

    Coord2D mRel; // Relative position
    Coord2D mAbs; // Absolute position only used for rendering
    bool mConnected = false; // Whether it's connected
    NodeSocket* mConnectedTo[MAX_SOCKET_CONNECTIONS] = { nullptr };
    // Buffer is only relevant for a output node

    /**
     * The audio buffer
     * if mIsInput is true it will contain the buffer to use as an input
     * if mIsInput is false the result will be stored there
     */
    sample** mBuffer = EMPTY_BUFFER;

    /**
     * Will make sure there are no empty spaces in the array and update the hasConnection flag
     */
    void sortConnectedTo() {
      bool hasConnection = false;
      for (int i = MAX_SOCKET_CONNECTIONS - 1; i >= 0 ; i--) {
        if (mConnectedTo[i] != nullptr) {
          hasConnection = true;
          for (int j = 0; j < i; j++) {
            if (mConnectedTo[j] == nullptr && i != j) {
              mConnectedTo[j] = mConnectedTo[i];
              mConnectedTo[i] = nullptr;
            }
          }
        }
      }
      mConnected = hasConnection;
      if (!mConnected && mIsInput) {
        // Reset the buffer back to a dummy if it's an input node to ensure the dsp won't crash
        mBuffer = EMPTY_BUFFER;
      }
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
  };
}