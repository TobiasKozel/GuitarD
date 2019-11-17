#pragma once

// Some structs used to pass around bundled data with the MessageBus

struct Coord2d {
  float x;
  float y;
};


class Node;
class NodeSocket;
struct NodeSpliceInPair {
  Node* node;
  NodeSocket* socket;
};

struct GraphStats {
  long long executionTime;
  int nodeCount;
};

struct SocketConnectRequest {
  NodeSocket* from;
  NodeSocket* to;
};