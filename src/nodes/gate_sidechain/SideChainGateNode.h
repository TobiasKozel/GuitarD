#pragma once
#include "../../faust/generated/SideChainGate.h"

namespace guitard {
  class SideChainGateNode final : public FaustGenerated::SideChainGate {
  public:
    SideChainGateNode(NodeList::NodeInfo* info) {
      mInfo = info;
    }
  };

  // GUITARD_REGISTER_NODE(SideChainGateNode, "Sidechain Gate", "Dynamics", "second input is the trigger")
}