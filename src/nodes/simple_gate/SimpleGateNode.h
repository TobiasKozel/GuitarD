#pragma once
#include "SimpleGate.h"

class SimpleGateNode : public SimpleGate {
public:
  SimpleGateNode(std::string pType) {
    type = pType;
  }
};