#pragma once
#include "SimpleComressor.h"

class SimpleComressorNode : public SimpleComressor {
public:
  SimpleComressorNode(std::string pType) {
    type = pType;
  }
};