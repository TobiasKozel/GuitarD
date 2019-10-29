#pragma once
#include "ParametricEq.h"

class ParametricEqNode : public ParametricEq {
public:
  ParametricEqNode(std::string pType) {
    type = pType;
  }
};