#pragma once

// Some structs used to pass around bundled data with the MessageBus

struct Coord2D {
  float x = 0;
  float y = 0;
};


class Node;
struct NodeSocket;

struct NodeSpliceInPair {
  Node* node = nullptr;
  NodeSocket* socket = nullptr;
};

struct GraphStats {
  long long executionTime = 0;
  int nodeCount = 0;
};

struct SocketConnectRequest {
  NodeSocket* from = nullptr;
  NodeSocket* to = nullptr;
};

struct ConnectionDragData {
  bool dragging = false;
  float startY = 0;
  float startX = 0;
  float currentY = 0;
  float currentX = 0;
};

struct AutomationAttachRequest {
  Node* automationNode = nullptr;
  iplug::igraphics::IControl* targetControl = nullptr;
};