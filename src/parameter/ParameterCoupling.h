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
    Milliseconds,
    Percentage
  };

  Type type = Auto;

  // Param object for outside daw automation
  iplug::IParam* parameter = nullptr;

  // Index of the Param
  int parameterIdx = -1;

  // pointer to the value used in the dsp code
  double* value = nullptr;

  // this will be added on top of the actual value to allow internal automation eventually
  double automation = 0;

  // This value is only used for params which couldn't claim a DAW parameter to act as the one provided by the IParam
  double baseValue = 0;

  double mAdd = 0;
  double mMul = 0;

  // Bounds and so on 
  double min = 0;
  double max = 1;
  double defaultVal = 0;
  double stepSize = 0.01;

  // Control object which will draw the UI knob
  iplug::igraphics::IControl* control = nullptr;
  // UI position and size
  float x = 0;
  float y = 0;
  float w = 60;
  float h = 60;

  // name which may be used in the UI
  const char* name;
  // custom image or something, prolly doesn't belong here
  const char* asset = nullptr;

  bool showLabel = true;
  bool showValue = true;

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

  template <typename T>
  T getValue() {
    if (parameter != nullptr) {
      return static_cast<T>(parameter->Value());
    }
    else {
      return static_cast<T>(baseValue);
    }
  }

  template <typename T>
  T getWithAutomation() {
    if (parameter != nullptr) {
      return static_cast<T>(parameter->Value() + automation);
    }
    else {
      return static_cast<T>(baseValue + automation);
    }
  }

  /**
   * This should only be called from the audio thread since the value might tear on 32bit
   */
  void update() const {
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