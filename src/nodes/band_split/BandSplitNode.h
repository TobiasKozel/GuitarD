#pragma once
#include "../../faust/generated/BandSplit.h"

namespace guitard {
  class BandSplitNode final : public FaustGenerated::BandSplit {
  public:
    BandSplitNode(NodeList::NodeInfo* info) {
      mInfo = info;
    }
  };

  GUITARD_REGISTER_NODE(BandSplitNode,
    "Band Split", "Signal Flow",
    "Splits up a signal into three frequency bands with Butterworth High/Lowpass filters"
  )
}
