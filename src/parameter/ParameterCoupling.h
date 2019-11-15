#pragma once
#include "IPlugParameter.h"


/**
 * Struct/Class used to pair up a daw IParam if one is available and always a IControl + dsp parameter to be altered
 * Also contains the value bounds, stepsize, name and IParam index
 * A ParameterCoupling can also exist wihtout a IParam which basically allows an unlimited
 * amount of IControls
 */
struct ParameterCoupling {
  enum Type {
    Auto,
    Linear,
    Boolean,
    Frequency,
    Gain,
    Seconds,
    Miliseconds,
    Percentage
  };
  Type type;
  // Param object for outside daw automation
  iplug::IParam* parameter;
  // Index of the Param
  int parameterIdx;
  // pointer to the value used in the dsp code
  double* value;

  // this will be added on top of the actual value to allow internal automation eventually
  double automation;
  // This value is only used for params which coudln't claim a DAW parameter
  double baseValue;

  // Bounds and so on 
  double min;
  double max;
  double defaultVal;
  double stepSize;

  // Control object which will draw the UI knob
  iplug::igraphics::IControl* control;
  // UI position and size
  float x;
  float y;
  float w;
  float h;
  // name which may be used in the UI
  const char* name;
  // custom image or something, prolly doesn't belong here
  const char* asset;

  ParameterCoupling(const char* p_name = nullptr, double* p_proprety = nullptr,
    double p_default = 0.5, double p_min = 0, double p_max = 1, double p_stepSize = 0.01, Type p_type = Auto) {
    value = p_proprety;
    defaultVal = p_default;
    min = p_min;
    max = p_max;
    stepSize = p_stepSize;
    name = p_name;
    parameterIdx = iplug::kNoParameter;
    parameter = nullptr;
    control = nullptr;
    baseValue = automation = 0;
    x = y = 0;
    asset = nullptr;
    w = h = 60;

    // Do some assumptions for the correct type
    if (p_type == Auto) {
      if (max == 20000) {
        type = Frequency;
      }
      else if (min == 0 && max == 1 && stepSize == 1) {
        type = Boolean;
      }
      else if (max >= 0 && min > -130 && min < -40) {
        type = Gain;
      }
      else if (min == 0 && max == 100) {
        type = Percentage;
      }
      else {
        type = Linear;
      }
    }
    else {
      type = p_type;
    }
  }

  /**
   * This should only be called from the audio thread since the value might tear on 32bit
   */
  void update() {
    if (parameter != nullptr) {
      *value = parameter->Value() + automation;
    }
    else {
      // maybe handle nonlinear scalings base don type here
      *value = baseValue + automation;
    }
  }
};