#pragma once
#include "src/misc/MessageBus.h"
#include "src/misc/GStructs.h"
#include "ptrlist.h"
#include "src/parameter/MeterCoupling.h"
#include "src/parameter/ParameterCoupling.h"

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

  WDL_PtrList<NodeSocket> socketsIn;
  WDL_PtrList<NodeSocket> socketsOut;
  Node* node = nullptr;
};