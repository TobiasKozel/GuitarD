#pragma once

#include "constants.h"
#include "json.hpp"

// TODOG make this instance specific so multiple plugins don't share the same undo stack
class HistoryStack {
  nlohmann::json* states[MAX_UNDOS] = { nullptr };
  int mIndex = 0;
  int mUndos = 0;
  int mRedos = 0;

public:
  void ClearStack() {
    for (int i = 0; i < MAX_UNDOS; i++) {
      if (states[i] != nullptr) {
        delete states[i];
      }
    }
    mUndos = mRedos = mIndex = 0;
  }

  nlohmann::json* pushState() {
    nlohmann::json* state = states[mIndex];
    if (state != nullptr) {
      delete state;
    }
    states[mIndex] = state = new nlohmann::json();

    mRedos = 0;
    mUndos++;
    if (mUndos >= MAX_UNDOS) {
      mUndos = MAX_UNDOS;
    }
    mIndex++;
    if (mIndex >= MAX_UNDOS) {
      mIndex = 0;
    }
    return state;
  }

  nlohmann::json* popState(const bool redo = false) {
    if (!redo) {
      // A PushState has to happen here for the redo to work
      // redos++;
      if (mUndos > 0) {
        mIndex--;
        if (mIndex < 0) {
          mIndex = MAX_UNDOS - 1;
        }
        mUndos--;
        return states[mIndex];
      }
      else {
        // Nothing to undo
        return nullptr;
      }
    }
    else {
      if (mRedos > 0) {
        mIndex++;
        if (mIndex >= MAX_UNDOS) {
          mIndex = 0;
        }
        return states[mIndex];
      }
      else {
        // Nothing to redo
        return nullptr;
      }
    }
  }
};
