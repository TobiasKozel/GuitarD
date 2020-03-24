#pragma once
#include "../../main/faust/generated/CryBaby.h"

namespace guitard {
  using CryBabyNode = FaustGenerated::CryBaby;
  GUITARD_REGISTER_NODE(CryBabyNode, "Crybaby", "Filters", "Wah!")
}
