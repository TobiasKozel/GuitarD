#pragma once
#include "../../main/faust/generated/StereoTool.h"

namespace guitard {
  using StereoToolNode = FaustGenerated::StereoTool;
  GUITARD_REGISTER_NODE(StereoToolNode,
    "Stereo Tool", "Tools", "Allows panning and altering the stereo width"
  )
}