#pragma once
#include "../../main/faust/generated/Split.h"

namespace guitard {
  using SplitNode = FaustGenerated::Split;

  GUITARD_REGISTER_NODE(SplitNode,
    "Split L/R", "Signal Flow", "Splits a signal into Left/Right and Mid/Side channels"
  )
}
