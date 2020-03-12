#pragma once
#include "../../faust/generated/SimpleGate.h"

namespace guitard {
  class SimpleGateNode final : public FaustGenerated::SimpleGate {
  public:
    SimpleGateNode(NodeList::NodeInfo* info) {
      mInfo = info;
    }
  };

  GUITARD_REGISTER_NODE(SimpleGateNode, "Basic Noise Gate", "Dynamics", "A nois gate")
}
