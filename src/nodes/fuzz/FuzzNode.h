#pragma once
#include "../../faust/generated/Fuzz.h"

namespace guitard {
  class FuzzNode final : public FaustGenerated::Fuzz {
  public:
    FuzzNode(NodeList::NodeInfo* info) {
      mInfo = info;
    }

    String getLicense() override {
      String l = "\nFaust code from Guitarix, probably needs to be replaced/removed";
      l += Fuzz::getLicense();
      return l;
    }
  };

  GUITARD_REGISTER_NODE(FuzzNode, "Fuzz", "Distortion", "apparently fuzz", "", true)
}