#pragma once
#include "../../main/faust/generated/SimpleComressor.h"

namespace guitard {
  class SimpleComressorNode final : public FaustGenerated::SimpleComressor {
  public:
    SimpleComressorNode(NodeList::NodeInfo* info) {
      mInfo = info;
    }
  };

  GUITARD_REGISTER_NODE(SimpleComressorNode, "Basic Compressor", "Dynamics", "does compressor things")
}
