#pragma once
#include "IPlugParameter.h"


/**
 * Struct/Class used to pair up a daw IParam if one is available and always a IControl + dsp parameter to be altered
 * Also contains the value bounds, step size, name and IParam index
 * A ParameterCoupling can also exist without a IParam which basically allows an unlimited
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
  // This value is only used for params which couldn't claim a DAW parameter to act as the one provided by the IParam
  double baseValue;

  double mAdd;
  double mMul;

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

  bool showLabel;
  bool showValue;

  explicit ParameterCoupling(const char* pName = nullptr, double* pProperty = nullptr,
                    const double pDefault = 0.5, const double pMin = 0, const double pMax = 1,
                    const double pStepSize = 0.01, const Type pType = Auto) {
    value = pProperty;
    defaultVal = pDefault;
    baseValue = pDefault;
    min = pMin;
    max = pMax;
    stepSize = pStepSize;
    name = pName;
    parameterIdx = iplug::kNoParameter;
    parameter = nullptr;
    parameter = nullptr;
    control = nullptr;
    automation = 0;
    x = y = 0;
    asset = nullptr;
    w = h = 60;
    showLabel = true;
    showValue = true;


    // Do some assumptions for the correct type
    if (pType == Auto) {
      if (max == 20000) {
        type = Frequency;
        if (min <= 0.) {
          min = 0.00000001;
        }
        mAdd = std::log(min);
        mMul = std::log(max / min);
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
      type = pType;
    }
  }

  void setPos(const float pX, const float pY, const float size = -1, const bool pShowLabel = true, const bool pShowValue = true) {
    x = pX;
    y = pY;
    if (size > 0) {
      w = h = size;
    }
    showLabel = pShowLabel;
    showValue = pShowValue;
  }

  /**
   * This should only be called from the audio thread since the value might tear on 32bit
   */
  void update() const
  {
    if (parameter != nullptr) {
      *value = parameter->Value() + automation;
    }
    else {
      // TODOG handle the other types here as well
      if (type == Frequency) {
        *value = std::exp(mAdd + ((baseValue - min) / max) * mMul) + automation;
      }
      else {
        *value = baseValue + automation;
      }
    }
  }
};