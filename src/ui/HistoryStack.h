#pragma once

#include "../GConfig.h"
#include "../../thirdparty/soundwoofer/dependencies/json.hpp"

#define GUITARD_MAX_UNDOS 8

namespace  guitard {
  // TODOG make this instance specific so multiple plugins don't share the same undo stack
  class HistoryStack {
    nlohmann::json* states[GUITARD_MAX_UNDOS] = { nullptr };
    int mIndex = 0;
    int mUndos = 0;
    int mRedos = 0;

  public:
    void ClearStack() {
      for (int i = 0; i < GUITARD_MAX_UNDOS; i++) {
        if (states[i] != nullptr) {
          delete states[i];
          states[i] = nullptr;
        }
      }
      mUndos = mRedos = mIndex = 0;
    }

    /**
     * Will create a new json object and put it on the stack
     * The state has to be stored in the returned object
     */
    nlohmann::json* pushState() {
      nlohmann::json* state = states[mIndex];
      delete state;
      states[mIndex] = state = new nlohmann::json();

      mRedos = 0;
      mUndos++;
      if (mUndos >= GUITARD_MAX_UNDOS) {
        mUndos = GUITARD_MAX_UNDOS;
      }
      mIndex++;
      if (mIndex >= GUITARD_MAX_UNDOS) {
        mIndex = 0;
      }
      return state;
    }

    /**
     * Will return a state if there is one
     */
    nlohmann::json* popState(const bool redo = false) {
      if (!redo) {
        // A PushState has to happen here for the redo to work
        // redos++;
        if (mUndos > 0) {
          mIndex--;
          if (mIndex < 0) {
            mIndex = GUITARD_MAX_UNDOS - 1;
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
          if (mIndex >= GUITARD_MAX_UNDOS) {
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
}
