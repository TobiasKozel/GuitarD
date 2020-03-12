#pragma once
#include "../../faust/generated/CryBaby.h"

namespace guitard {
  class CryBabyNode final : public FaustGenerated::CryBaby {
  public:
    CryBabyNode(NodeList::NodeInfo* info) {
      mInfo = info;
    }
  };

  GUITARD_REGISTER_NODE(CryBabyNode, "Crybaby", "Filters", "Wah!")
}
