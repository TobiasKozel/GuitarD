#pragma once
#include "../../main/faust/generated/SimpleReverb.h"

namespace guitard {
  using SimpleReverbNode = FaustGenerated::SimpleReverb;
  GUITARD_REGISTER_NODE(SimpleReverbNode, "Stereo Reverb", "Delays/Reverbs", "Zika Reverb")
}
