#pragma once
#include "../misc/MessageBus.h"
#include "../types/gstructs.h"
#include "../parameter/MeterCoupling.h"
#include "../parameter/ParameterCoupling.h"

namespace guitard {
  /**
   * A struct of data that is shared between the node and its UI
   */
  struct NodeShared {
    std::string type;
    MessageBus::Bus* bus = nullptr;
#ifndef GUITARD_HEADLESS
    IGraphics* graphics = nullptr; // TODOG Get rid of this
#endif
    float width = 250;
    float height = 180;
    float X = 0;
    float Y = 0;
    float rotation = 0;
    int maxBlockSize = 0;

    int parameterCount = 0;
    ParameterCoupling parameters[MAX_NODE_PARAMETERS];
    int meterCount = 0;
    MeterCoupling* meters[MAX_NODE_METERS] = { nullptr };

    int inputCount = 0;
    NodeSocket* socketsIn[MAX_NODE_SOCKETS] = { nullptr };
    int outputCount = 0;
    NodeSocket* socketsOut[MAX_NODE_SOCKETS] = { nullptr };
    Node* node = nullptr;
  };
}