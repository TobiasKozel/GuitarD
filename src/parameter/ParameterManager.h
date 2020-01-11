#pragma once

#include "IPlugParameter.h"
#include "src/misc/constants.h"
#include "src/node/Node.h"
#include "src/parameter/ParameterCoupling.h"
#include "src/misc/MessageBus.h"

class ParameterManager {
  MessageBus::Bus* mBus;
  iplug::IParam* mParameters[MAX_DAW_PARAMS] = { nullptr };
  bool mParametersClaimed[MAX_DAW_PARAMS] = { true };
  int mParametersLeft = 0;

public:
  explicit ParameterManager(MessageBus::Bus* pBus) {
    mBus = pBus;
  }

  /**
   * This will fill the pool of parameters from the DAW ONCE at plugin startup
   * since most DAWs don't seem to support dynamic parameters
   */
  void addParameter(iplug::IParam* param) {
    std::string paramprefix = "Uninitialized ";
    mParametersClaimed[mParametersLeft] = false;
    mParameters[mParametersLeft++] = param;
    // all these values have a range from 0-1 since this can't be changed later on
    param->InitDouble((paramprefix + std::to_string(mParametersLeft)).c_str(), 1, 0, 1.0, 0.01);
  }

  /**
   * This will go over each control element in the paramters array of the node and try to expose it to the daw
   * Will return true if all parameters could be claimed, false if at least one failed
   */
  bool claimNode(Node* node) {
    bool gotAllParams = true;
    std::string prefix = NodeList::getInfo(node->shared.type)->displayName;
    for (int i = 0; i < node->shared.parameterCount; i++) {
      if (!claimParameter(&node->shared.parameters[i], prefix.c_str())) {
        /**
         * this means the manager has no free parameters left and the control cannot be automated from the daw
         */
        WDBGMSG("Ran out of daw parameters!\n");
        gotAllParams = false;
      }
    }
    MessageBus::fireEvent<bool>(mBus, MessageBus::ParametersChanged, false);
    return gotAllParams;
  }

  /**
   * This will provide one of the reserved daw parameters. If one is free, it will return true
   */
  bool claimParameter(ParameterCoupling* couple, const char* prefix = nullptr) {
    const char* name = couple->name;
    std::string stringName;
    if (prefix != nullptr) {
      /**
       * In case we get a node name as a prefix use that but keep the string object around until
       * the init function has copied it into the daw land
       */
      stringName = std::string(prefix) + " " + std::string(name);
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
    if (MAX_DAW_PARAMS <= i || mParametersClaimed[i] || i == kNoParameter || !couple->wantsDawParameter) {
      // This is bad and means a preset will not load correctly
      couple->setParam(nullptr);
      couple->parameterIdx = kNoParameter;
      if (couple->wantsDawParameter) {
        // Couldn't get a param but wanted one
        WDBGMSG("Could not claim a prefered DAW parameter!\n");
        return false;
      }
      return true;
    }
    mParametersLeft--;
    mParametersClaimed[i] = true;
    couple->setParam(mParameters[i]);
    couple->parameterIdx = i;
    WDBGMSG("Claimed param %i\n", i);
    return true;
  }

  void releaseNode(Node* node) {
    for (int i = 0; i < node->shared.parameterCount; i++) {
      releaseParameter(&node->shared.parameters[i]);
    }
    MessageBus::fireEvent<bool>(mBus, MessageBus::ParametersChanged, false);
  }

  void releaseParameter(ParameterCoupling* couple) {
    for (int i = 0; i < MAX_DAW_PARAMS; i++) {
      if (mParameters[i] == couple->getParam()) {
        mParametersClaimed[i] = false;
        std::string paramprefix = "Uninitialized ";
        mParameters[i]->InitDouble(
          (paramprefix + std::to_string(mParametersLeft)).c_str(), 1, 0, 1.0, 0.01
        );
        couple->setParam(nullptr);
        mParametersLeft++;
        WDBGMSG("Released param %i\n", i);
        return;
      }
    }
  }
};
