#pragma once

#include "src/constants.h"
#include "IPlugParameter.h"

struct ParameterCoupling {
  iplug::IParam* parameter;
  double* value;

  ParameterCoupling() {
    value = nullptr;
    parameter = nullptr;
  }

  void update() {
    double prev = *value;
    double paramVal = parameter->Value();
    *value = paramVal;
    double post = *value;
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

  ParameterCoupling* claimParameter(double* value, const char* name) {
    ParameterCoupling* ret = new ParameterCoupling;
    if (parametersLeft > 0) {
      parametersLeft--;
      for (int i = 0; i < MAXDAWPARAMS; i++) {
        if (!parametersClaimed[i]) {
          parametersClaimed[i] = true;
          ret->parameter = parameters[i];
          ret->value = value;
          ret->parameter->InitDouble(name, *value, 0, 127, 1);
          ret->parameter->SetLabel(name);
          ret->parameter->SetDisplayText(1, name);
          break;
        }
      }
    }
    return ret;
  }

  void releaseParameter(ParameterCoupling* param) {
    for (int i = 0; i < MAXDAWPARAMS; i++) {
      if (parameters[i] == param->parameter) {
        parametersClaimed[i] = false;
        parameters[i]->SetLabel("Released");
        parameters[i]->SetDisplayText(1, "Released");
        // memleak TODO check if the const char cause leaks
        delete param;
        break;
      }
    }
  }
};