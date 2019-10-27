#pragma once
#include "BitCrusher.h"

class BitCrusherNode : public BitCrusher {
public:
  BitCrusherNode(std::string pType) {
    type = pType;
  }
};