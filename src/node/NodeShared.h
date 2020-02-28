#pragma once
#include "../misc/MessageBus.h"
#include "../types/gstructs.h"
#include "../parameter/MeterCoupling.h"
#include "../parameter/ParameterCoupling.h"
#include "NodeInfo.h"

namespace guitard {
  /**
   * A struct of data that is shared between the node and its UI
   */
  struct NodeShared {
     // Only a pointer to the node info to avoid allocations
    MessageBus::Bus* bus = nullptr;
    float width = 250;
    float height = 180;
    float rotation = 0;
  };
}
