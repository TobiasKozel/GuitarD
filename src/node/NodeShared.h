#pragma once
#include "src/misc/MessageBus.h"
#include "src/misc/GStructs.h"
#include "ptrlist.h"
#include "src/parameter/MeterCoupling.h"
#include "src/parameter/ParameterCoupling.h"

/**
 * A struct of data that is shared between the node and its UI
 */
struct NodeShared {
  MessageBus::Bus* bus = nullptr;
  iplug::igraphics::IGraphics* graphics = nullptr;
  float width = 250;
  float height = 180;
  float X;
  float Y;
  float rotation = 0;
  WDL_PtrList<ParameterCoupling> parameters;
  WDL_PtrList<MeterCoupling> meters;

  int inputCount = 0;
  NodeSocket* socketsIn [MAX_NODE_SOCKETS];
  int outputCount = 0;
  NodeSocket* socketsOut[MAX_NODE_SOCKETS];
  Node* node = nullptr;
};