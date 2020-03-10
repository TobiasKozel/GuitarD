#pragma once
#include "../../faust/generated/Phaser.h"

namespace guitard {
  class PhaserNode final : public FaustGenerated::Phaser {
  public:
    explicit PhaserNode(NodeList::NodeInfo* info) {
      mInfo = info;
      mDimensions.x = 250;
      mDimensions.y = 240;
    }
  };

  GUITARD_REGISTER_NODE(PhaserNode, "Phaser", "Filters", "Phaser effect", "image")
}
