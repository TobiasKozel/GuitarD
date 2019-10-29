#pragma once


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