#pragma once
#include "../../main/faust/generated/OverDrive.h"
namespace guitard {
  using OverDriveNode = FaustGenerated::OverDrive;
  GUITARD_REGISTER_NODE(OverDriveNode, "Overdrive", "Distortion", "I drive.")
}