#pragma once
#include "../../main/faust/generated/SimpleComressor.h"

namespace guitard {
  using SimpleComressorNode = FaustGenerated::SimpleComressor;
  GUITARD_REGISTER_NODE(SimpleComressorNode, "Basic Compressor", "Dynamics", "does compressor things")
}
