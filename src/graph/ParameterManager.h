#pragma once

#include "src/constants.h"
#include "IPlugParameter.h"
#include "IControl.h"


/**
 * Struct/Class used to pair up a daw IParam if one is available and always a IControl + dsp parameter to be altered
 * Also contains the value bounds, stepsize, name and IParam index
 */
struct ParameterCoupling {
  iplug::IParam* parameter;
  iplug::igraphics::IControl* control;
  int parameterIdx;
  double* value;
  double min;
  double max;
  double defaultVal;
  double stepSize;
  const char* name;

  ParameterCoupling(const char* p_name = nullptr, double* p_proprety = nullptr,
     double p_default = 0.5, double p_min = 0, double p_max = 1, double p_stepSize = 0.01) {
    value = p_proprety;
    defaultVal = p_default;
    min = p_min;
    max = p_max;
    stepSize = p_stepSize;
    name = p_name;
    parameterIdx = iplug::kNoParameter;
    parameter = nullptr;
    control = nullptr;
  }

  /**
   * This should only be called from the audio thread since the value might tear on 32bit
   */
  void update() {
    if (parameter != nullptr) {
      *value = parameter->Value();
    }
    else {
    }
  }
};

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
    // all these values have a range from 0-100 since this can't be changed later on
    // TODO find out how scaled paramters work
    param->InitDouble((paramprefix + std::to_string(parametersLeft)).c_str(), 1, 0, 100.0, 0.1);
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
          WDBGMSG("Could not claim a prefered DAW parameter!");
          // This is bad
          couple->parameter = nullptr;
          couple->parameterIdx = iplug::kNoParameter;
          return false;
        }
      }
      parametersLeft--;
      parametersClaimed[i] = true;
      couple->parameter = parameters[i];
      couple->parameter->InitDouble(
        couple->name, couple->defaultVal, couple->min, couple->max, couple->stepSize
      );
      couple->parameter->SetLabel(couple->name);
      couple->parameter->SetDisplayText(1, couple->name);
      couple->parameterIdx = i;
      couple->parameter->Set(*(couple->value));
      return true;
    }
    // no free parameters left
    couple->parameter = nullptr;
    couple->parameterIdx = iplug::kNoParameter;
    return false;
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
        return;
      }
    }
  }
};
