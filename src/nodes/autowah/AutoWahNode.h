#pragma once
#include "../../faust/generated/AutoWah.h"

namespace guitard {
  class AutoWahNode final : public FaustGenerated::AutoWah {
  public:
    explicit AutoWahNode(NodeList::NodeInfo* info) {
      mInfo = info;
      mDimensions.x = 200;
      mDimensions.y = 200;
    }
  };

  GUITARD_REGISTER_NODE(AutoWahNode, "Auto Wah", "Filters", "Wah!")
}
