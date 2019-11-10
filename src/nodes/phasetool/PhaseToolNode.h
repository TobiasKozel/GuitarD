#pragma once
#include "PhaseTool.h"

class PhaseToolNode : public PhaseTool {
 public:
  PhaseToolNode(std::string pType) {
    type = pType;
  }
};