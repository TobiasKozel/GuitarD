#pragma once
#include "../../main/faust/generated/SimpleDelay.h"

namespace guitard {
  class SimpleDelayNode final : public FaustGenerated::SimpleDelay {
  public:
    SimpleDelayNode() {
      mDimensions.x = 300;
    }
  };

  GUITARD_REGISTER_NODE(SimpleDelayNode,
    "Basic Delay", "Delays/Reverbs", "A very simple delay effect"
  )
}
