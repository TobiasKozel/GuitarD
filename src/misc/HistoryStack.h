#pragma once

#include "constants.h"
#include "thirdparty/json.hpp"
#include "ptrlist.h"

using namespace std;
using namespace nlohmann;
// TODOG make this instance specific so multiple plugins don't share the same undo stack
namespace HistoryStack {
  json* states[MAX_UNDOS] = { nullptr };
  int index = 0;
  int undos = 0;
  int redos = 0;

  void ClearStack() {
    for (int i = 0; i < MAX_UNDOS; i++) {
      if (states[i] != nullptr) {
        delete states[i];
      }
    }
    undos = redos = index = 0;
  }

  json* PushState() {
    json* state = states[index];
    if (state != nullptr) {
      delete state;
    }
    states[index] = state = new json();

    redos = 0;
    undos++;
    if (undos >= MAX_UNDOS) {
      undos = MAX_UNDOS;
    }
    index++;
    if (index >= MAX_UNDOS) {
      index = 0;
    }
    return state;
  }

  json* PopState(bool redo = false) {
    if (!redo) {
      // A PushState has to happen here for the redo to work
      // redos++;
      if (undos > 0) {
        index--;
        if (index < 0) {
          index = MAX_UNDOS - 1;
        }
        undos--;
        return states[index];
      }
      else {
        // Nothing to undo
        return nullptr;
      }
    }
    else {
      if (redos > 0) {
        index++;
        if (index >= MAX_UNDOS) {
          index = 0;
        }
        return states[index];
      }
      else {
        // Nothing to redo
        return nullptr;
      }
    }
  }
}