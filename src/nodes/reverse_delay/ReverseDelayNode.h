#pragma once
#include "../../main/faust/generated/ReverseDelay.h"

namespace guitard {
  class ReverseDelayNode final : public FaustGenerated::ReverseDelay {
  public:
    ReverseDelayNode() {
      mDimensions.x = 300;
    }

    String getLicense() override {
      String l = "Code from https://gist.github.com/tomoyanonymous\n";
      l += "Gist https://gist.github.com/tomoyanonymous/d527fca58e929de6a021565505589406\n";
      l += "No license provided, need to ask for permission";
      l += ReverseDelay::getLicense();
      return l;
    }
  };

  GUITARD_REGISTER_NODE(
    ReverseDelayNode, "Reverse Delay", "Delays/Reverbs",
    "Reversed Delay effect (Kinda Clicky)"
  )
}
