#pragma once
#include "../../faust/generated/Transpose.h"

namespace guitard {
  class TransposeNode final : public FaustGenerated::Transpose {
  public:
    TransposeNode(NodeList::NodeInfo* info) {
      mInfo = info;
    }
  };

  GUITARD_REGISTER_NODE(TransposeNode, "Transpose", "Filters", "Simple Transposed", "image")
}
