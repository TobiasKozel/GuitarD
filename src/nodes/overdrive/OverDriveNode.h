#pragma once
#include "../../faust/generated/OverDrive.h"
namespace guitard {
  class OverDriveNode final : public FaustGenerated::OverDrive {
  public:
    OverDriveNode(NodeList::NodeInfo* info) {
      mInfo = info;
    }
  };

  GUITARD_REGISTER_NODE(OverDriveNode,
    "Overdrive", "Distortion", "I drive.", "image"
  )
}