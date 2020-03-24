#pragma once
#include "../../main/faust/generated/SimpleGate.h"

namespace guitard {
  using SimpleGateNode = FaustGenerated::SimpleGate;
  GUITARD_REGISTER_NODE(SimpleGateNode, "Basic Noise Gate", "Dynamics", "A nois gate")
}
