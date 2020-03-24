#pragma once
#include "../../main/faust/generated/Flanger.h"

namespace guitard {
  using FlangerNode = FaustGenerated::Flanger;

  GUITARD_REGISTER_NODE(FlangerNode, "Flanger", "Filters", "Flange effect")
}
