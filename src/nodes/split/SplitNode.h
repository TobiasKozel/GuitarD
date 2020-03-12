#pragma once
#include "../../faust/generated/Split.h"

namespace guitard {
  class SplitNode final : public FaustGenerated::Split {
  public:
    SplitNode(NodeList::NodeInfo* info) {
      mInfo = info;
    }
  };

  GUITARD_REGISTER_NODE(SplitNode,
    "Split L/R", "Signal Flow", "Splits a signal into Left/Right and Mid/Side channels"
  )
}
