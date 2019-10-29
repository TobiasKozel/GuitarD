#pragma once
#include "SimpleReverb.h"

class SimpleReverbNode : public SimpleReverb {
public:
  SimpleReverbNode(std::string pType) {
    type = pType;
  }
};
