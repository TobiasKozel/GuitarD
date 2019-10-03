#pragma once

#include "src/constants.h"
#include "IPlugParameter.h"

struct ParameterCoupling {
  iplug::IParam* parameter;
  double* value;
  double min, max, default, stepSize;
  const char* name;

  ParameterCoupling(const char* p_name = nullptr, double* p_proprety = nullptr,
     double p_default = 0.5, double p_min = 0, double p_max = 1, double p_stepSize = 0.1) {
    value = p_proprety;
    default = p_default;
    min = p_min;
    max = p_max;
    stepSize = p_stepSize;
    name = p_name;
    parameter = nullptr;
  }

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

  void addParameter(iplug::IParam* param) {
    // note, only do this on the init
    std::string paramprefix = "Uninitialized ";
    parametersClaimed[parametersLeft] = false;
    parameters[parametersLeft++] = param;
    param->InitDouble((paramprefix + std::to_string(parametersLeft)).c_str(), 1, 0, 100, 1);
  }

  /**
   * This will provide one of the reserved daw parameters. If one is free, it will return true
  */
  bool claimParameter(ParameterCoupling* couple) {
    if (parametersLeft > 0) {
      parametersLeft--;
      for (int i = 0; i < MAXDAWPARAMS; i++) {
        if (!parametersClaimed[i]) {
          parametersClaimed[i] = true;
          couple->parameter = parameters[i];
          couple->parameter->InitDouble(
            couple->name, *(couple->value), couple->min, couple->max, couple->stepSize
          );
          couple->parameter->SetLabel(couple->name);
          couple->parameter->SetDisplayText(1, couple->name);
          return true;
        }
      }
    }
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