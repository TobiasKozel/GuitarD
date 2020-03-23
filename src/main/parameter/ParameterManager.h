#pragma once

#include "../../GConfig.h"
#include "../Node.h"
#include "./ParameterCoupling.h"
#include <functional>

namespace guitard {
  class ParameterManager {
#ifndef GUITARD_HEADLESS
    IParam*
#else
    ParameterCoupling*
#endif
    mParameters[MAX_DAW_PARAMS] = { nullptr };

    bool mParametersClaimed[MAX_DAW_PARAMS] = { true };
    int mParametersLeft = 0;

    /**
     * If the parameters change the daw or whatever is in charge might want to know about that
     */
    std::function<void()> mCallback;
  public:
    explicit ParameterManager(std::function<void()> callback) {
      mCallback = callback;
    }

#ifndef GUITARD_HEADLESS
    /**
     * This will fill the pool of parameters from the DAW ONCE at plugin startup
     * since most DAWs don't seem to support dynamic parameters
     */
    void addParameter(IParam* param) {
      String paramprefix = "Uninitialized ";
      mParametersClaimed[mParametersLeft] = false;
      mParameters[mParametersLeft++] = param;
      // all these values have a range from 0-1 since this can't be changed later on
      param->InitDouble((paramprefix + std::to_string(mParametersLeft)).c_str(), 1, 0, 1.0, 0.01);
    }
#endif

    /**
     * This will go over each control element in the paramters array of the node and try to expose it to the daw
     * Will return true if all parameters could be claimed, false if at least one failed
     */
    bool claimNode(Node* node) {
      bool gotAllParams = true;
      String prefix = node->mInfo->displayName;
      for (int i = 0; i < node->mParameterCount; i++) {
        if (node->mParameters[i].wantsDawParameter && !claimParameter(&node->mParameters[i], prefix.c_str())) {
          /**
           * this means the manager has no free parameters left and the control cannot be automated from the daw
           */
          WDBGMSG("Ran out of daw parameters!\n");
          gotAllParams = false;
        }
      }

      mCallback();
      return gotAllParams;
    }

    /**
     * This will provide one of the reserved daw parameters. If one is free, it will return true
     */
    bool claimParameter(ParameterCoupling* couple, const char* prefix = nullptr) {
      const char* name = couple->name;
      String stringName;
      if (prefix != nullptr) {
        /**
         * In case we get a node name as a prefix use that but keep the string object around until
         * the init function has copied it into the daw land
         */
        stringName = String(prefix) + " " + String(name);
        name = stringName.c_str();
      }
      int i = couple->parameterIdx;
      if (i == kNoParameter && mParametersLeft > 0) {
        // if there's no parameter index set, go look for one
        for (i = 0; i < MAX_DAW_PARAMS; i++) {
          if (!mParametersClaimed[i]) {
            // found one
            break;
          }
        }
      }
      if (MAX_DAW_PARAMS <= i || mParametersClaimed[i] || i == kNoParameter) {
        // This is bad and means a preset will not load correctly
#ifndef GUITARD_HEADLESS
        couple->setParam(nullptr);
#endif
        couple->parameterIdx = kNoParameter;
        WDBGMSG("Could not claim a prefered DAW parameter!\n");
        return false;
      }
      mParametersLeft--;
      mParametersClaimed[i] = true;
#ifndef GUITARD_HEADLESS
      couple->setParam(mParameters[i]);
#else
      mParameters[i] = couple; // Store the couple here so it can be accessed from outside
#endif
      couple->parameterIdx = i;
      WDBGMSG("Claimed param %i\n", i);
      return true;
    }

    void releaseNode(Node* node) {
      for (int i = 0; i < node->mParameterCount; i++) {
        releaseParameter(&node->mParameters[i]);
      }
      mCallback();
    }

    void releaseParameter(ParameterCoupling* couple) {
      for (int i = 0; i < MAX_DAW_PARAMS; i++) {
#ifndef GUITARD_HEADLESS
        if (mParameters[i] == couple->getParam()) {
#else
        if (mParameters[i] == couple) { // directly compare the couple
#endif
          mParametersClaimed[i] = false;
          const String paramprefix = "Uninitialized ";
#ifndef GUITARD_HEADLESS
          mParameters[i]->InitDouble(
            (paramprefix + std::to_string(mParametersLeft)).c_str(), 1, 0, 1.0, 0.01
          );
          couple->setParam(nullptr);
#else
          mParameters[i] = nullptr;
#endif
          mParametersLeft++;
          WDBGMSG("Released param %i\n", i);
          return;
        }
      }
    }

#ifdef GUITARD_HEADLESS
    ParameterCoupling* getCoupling(int index) {
        if (index < MAX_DAW_PARAMS && mParameters[index] != nullptr) {
          return mParameters[index];
        }
        return nullptr;
      }
#endif
  };
}