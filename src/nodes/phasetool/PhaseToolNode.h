#pragma once
#include "../../main/faust/generated/PhaseTool.h"

namespace guitard {
  class PhaseToolNode final : public FaustGenerated::PhaseTool {
  public:
    PhaseToolNode(NodeList::NodeInfo* info) {
      mInfo = info;
    }
  };

  GUITARD_REGISTER_NODE(PhaseToolNode,
    "Phase Tool", "Tools", "Delays a signal"
  )
}