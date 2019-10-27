#pragma once
#include "Fuzz.h"

class FuzzNode : public Fuzz {
public:
  FuzzNode(std::string pType) {
    type = pType;
  }
};