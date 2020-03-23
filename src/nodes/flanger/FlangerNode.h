#pragma once
#include "../../main/faust/generated/Flanger.h"

namespace guitard {
  class FlangerNode final : public FaustGenerated::Flanger {
  public:
    FlangerNode(NodeList::NodeInfo* info) {
      mInfo = info;
    }
  };

  GUITARD_REGISTER_NODE(FlangerNode, "Flanger", "Filters", "Flange effect")
}
