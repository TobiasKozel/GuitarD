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
    for (int i = 0; i < node->mParameters.GetSize(); i++) {
      if (!claimParameter(node->mParameters.Get(i))) {
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
  bool claimParameter(ParameterCoupling* couple) {
    int i = couple->parameterIdx;
    if (i == iplug::kNoParameter && mParametersLeft > 0) {
      // if there's no parameter index set, go look for one
      for (i = 0; i < MAX_DAW_PARAMS; i++) {
        if (!mParametersClaimed[i]) {
          // found one
          break;
        }
      }
    }
    if (MAX_DAW_PARAMS <= i || mParametersClaimed[i] || i == iplug::kNoParameter) {
      WDBGMSG("Could not claim a prefered DAW parameter!\n");
      // This is bad and means a preset will not load correctly
      couple->parameter = nullptr;
      couple->parameterIdx = iplug::kNoParameter;
      // Set the base value to the one from the preset
      couple->baseValue = *(couple->value);
      return false;
    }
    mParametersLeft--;
    mParametersClaimed[i] = true;
    couple->parameter = mParameters[i];
    switch (couple->type)
    {
    case ParameterCoupling::Boolean:
      couple->parameter->InitBool(couple->name, couple->defaultVal == 1.0);
      break;
    case ParameterCoupling::Frequency:
      couple->parameter->InitFrequency(
        couple->name, couple->defaultVal, couple->min, couple->max, couple->stepSize
      );
      break;
    case ParameterCoupling::Gain:
      couple->parameter->InitGain(
        couple->name, couple->defaultVal, couple->min, couple->max, couple->stepSize
      );
      break;
    case ParameterCoupling::Percentage:
      couple->parameter->InitPercentage(
        couple->name, couple->defaultVal, couple->min, couple->max, couple->stepSize
      );
      break;
    case ParameterCoupling::Linear:
    default:
      couple->parameter->InitDouble(
        couple->name, couple->defaultVal, couple->min, couple->max, couple->stepSize
      );
      break;
    }
    // TODOG These seem to be leaking and also don't force vsts to update the names
    // works for AU though
    //couple->parameter->SetLabel(couple->name);
    //couple->parameter->SetDisplayText(1, couple->name);
    couple->parameterIdx = i;
    couple->parameter->Set(*(couple->value));
    WDBGMSG("Claimed param %i\n", i);
    return true;
  }

  void releaseNode(Node* node) {
    for (int i = 0; i < node->mParameters.GetSize(); i++) {
      releaseParameter(node->mParameters.Get(i));
    }
    MessageBus::fireEvent<bool>(mBus, MessageBus::ParametersChanged, false);
  }

  void releaseParameter(ParameterCoupling* couple) {
    for (int i = 0; i < MAX_DAW_PARAMS; i++) {
      if (mParameters[i] == couple->parameter) {
        mParametersClaimed[i] = false;
        mParameters[i]->SetLabel("Released");
        mParameters[i]->SetDisplayText(1, "Released");
        // memleak TODO check if the const char cause leaks
        couple->parameter = nullptr;
        mParametersLeft++;
        WDBGMSG("Released param %i\n", i);
        return;
      }
    }
  }
};
