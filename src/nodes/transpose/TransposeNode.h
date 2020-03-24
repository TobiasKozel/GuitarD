#pragma once
#include "../../main/faust/generated/Transpose.h"

namespace guitard {
  using TransposeNode = FaustGenerated::Transpose;
  GUITARD_REGISTER_NODE(TransposeNode, "Transpose", "Filters", "Simple Transposed")
}
