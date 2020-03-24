#pragma once
#include "../../main/faust/generated/SideChainGate.h"

namespace guitard {
  using SideChainGateNode = FaustGenerated::SideChainGate;
  GUITARD_REGISTER_NODE(SideChainGateNode, "Sidechain Gate", "Dynamics", "second input is the trigger")
}