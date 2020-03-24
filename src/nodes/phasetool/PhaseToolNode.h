#pragma once
#include "../../main/faust/generated/PhaseTool.h"

namespace guitard {
  using PhaseToolNode = FaustGenerated::PhaseTool;
  GUITARD_REGISTER_NODE(PhaseToolNode, "Phase Tool", "Tools", "Delays a signal")
}