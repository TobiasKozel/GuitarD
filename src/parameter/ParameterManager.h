#pragma once

#include "IPlugParameter.h"
#include "IControl.h"
#include "src/misc/constants.h"
#include "src/node/Node.h"
#include "src/parameter/ParameterCoupling.h"
#include "src/misc/MessageBus.h"

class ParameterManager {
  iplug::IParam* parameters[MAXDAWPARAMS];
  bool parametersClaimed[MAXDAWPARAMS];
  int parametersLeft;

public:
  ParameterManager() {
    parametersLeft = 0;
    for (int i = 0; i < MAXDAWPARAMS; i++) {
      parameters[i] = nullptr;
      parametersClaimed[i] = true;
    }
  }

  /**
   * This will fill the pool of parameters from the DAW ONCE at plugin startup
   * since most DAWs don't seem to support dynamic parameters
   */
  void addParameter(iplug::IParam* param) {
    std::string paramprefix = "Uninitialized ";
    parametersClaimed[parametersLeft] = false;
    parameters[parametersLeft++] = param;
    // all these values have a range from 0-1 since this can't be changed later on
    param->InitDouble((paramprefix + std::to_string(parametersLeft)).c_str(), 1, 0, 1.0, 0.01);
  }

  /**
   * This will go over each control element in the paramters array of the node and try to expose it to the daw
   * Will return true if all parameters could be claimed, false if at least one failed
   */
  bool claimNode(Node* node) {
    bool gotAllPamams = true;
    for (int i = 0; i < node->parameters.GetSize(); i++) {
      if (!claimParameter(node->parameters.Get(i))) {
        /**
         * this means the manager has no free parameters left and the control cannot be automated from the daw
         */
        WDBGMSG("Ran out of daw parameters!\n");
        gotAllPamams = false;
      }
    }
    MessageBus::fireEvent<bool>(MessageBus::ParametersChanged, false);
    return gotAllPamams;
  }

  /**
   * This will provide one of the reserved daw parameters. If one is free, it will return true
   */
  bool claimParameter(ParameterCoupling* couple) {
    if (parametersLeft > 0) {
      int i = couple->parameterIdx;
      if (i == iplug::kNoParameter) {
        // if there's no parameter index set, go look for one
        for (i = 0; i < MAXDAWPARAMS; i++) {
          if (!parametersClaimed[i]) {
            // found one
            break;
          }
        }
      } else {
        if (parametersClaimed[i]) {
          WDBGMSG("Could not claim a prefered DAW parameter!\n");
          // This is bad
          couple->parameter = nullptr;
          couple->parameterIdx = iplug::kNoParameter;
          return false;
        }
      }
      parametersLeft--;
      parametersClaimed[i] = true;
      couple->parameter = parameters[i];
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
      // TODO These seem to be leaking and also don't force vsts to update the names
      // works for AU though
      //couple->parameter->SetLabel(couple->name);
      //couple->parameter->SetDisplayText(1, couple->name);
      couple->parameterIdx = i;
      couple->parameter->Set(*(couple->value));
      WDBGMSG("Claimed param %i\n", i);
      return true;
    }
    // no free parameters left
    couple->parameter = nullptr;
    couple->parameterIdx = iplug::kNoParameter;
    return false;
  }

  void releaseNode(Node* node) {
    for (int i = 0; i < node->parameters.GetSize(); i++) {
      releaseParameter(node->parameters.Get(i));
    }
    MessageBus::fireEvent<bool>(MessageBus::ParametersChanged, false);
  }

  void releaseParameter(ParameterCoupling* couple) {
    for (int i = 0; i < MAXDAWPARAMS; i++) {
      if (parameters[i] == couple->parameter) {
        parametersClaimed[i] = false;
        parameters[i]->SetLabel("Released");
        parameters[i]->SetDisplayText(1, "Released");
        // memleak TODO check if the const char cause leaks
        couple->parameter = nullptr;
        parametersLeft++;
        WDBGMSG("Released param %i\n", i);
        return;
      }
    }
  }
};
