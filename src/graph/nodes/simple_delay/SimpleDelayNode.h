#pragma once
#include "SimpleDelay.h"

class SimpleDelayNode : public SimpleDelay {
public:
  SimpleDelayNode(std::string pType) {
    type = pType;
  }
};
