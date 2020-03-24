#pragma once
#include "../../main/faust/generated/Phaser.h"

namespace guitard {
  class PhaserNode final : public FaustGenerated::Phaser {
  public:
    PhaserNode() {
      mDimensions.x = 250;
      mDimensions.y = 240;
    }
  };

  GUITARD_REGISTER_NODE(PhaserNode, "Phaser", "Filters", "Phaser effect")
}
