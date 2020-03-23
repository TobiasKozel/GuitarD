#pragma once
#include "../../main/faust/generated/SimpleReverb.h"

namespace guitard {
  class SimpleReverbNode final : public FaustGenerated::SimpleReverb {
  public:
    SimpleReverbNode(NodeList::NodeInfo* info) {
      mInfo = info;
    }
  };

  GUITARD_REGISTER_NODE(SimpleReverbNode,
    "Stereo Reverb", "Delays/Reverbs",
    "Zika Reverb"
  )
}
