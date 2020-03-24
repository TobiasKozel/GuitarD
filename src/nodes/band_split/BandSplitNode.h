#pragma once
#include "../../main/faust/generated/BandSplit.h"

namespace guitard {
  using BandSplitNode = FaustGenerated::BandSplit;

  GUITARD_REGISTER_NODE(BandSplitNode,
    "Band Split", "Signal Flow",
    "Splits up a signal into three frequency bands with Butterworth High/Lowpass filters"
  )
}
