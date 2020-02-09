#pragma once

namespace guitard {
  // Some structs used to pass around bundled data with the MessageBus
  class Node;
  struct NodeSocket;

  struct Coord2D {
    float x = 0;
    float y = 0;
  };



  struct QuickConnectRequest {
    Coord2D pos;
    NodeSocket* from;
  };

  struct NodeSpliceInPair {
    Node* node = nullptr;
    NodeSocket* socket = nullptr;
  };

  struct GraphStats {
    long long executionTime = 0;
    int nodeCount = 0;
    bool valid = true;
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
#ifndef GUITARD_HEADLESS
    iplug::igraphics::IControl* targetControl = nullptr;
#endif
  };

  struct NodeDragEndData {
    Node* node;
    bool addCombineNode;
  };


  /**
   * Bundles an impulse response with some meta data about it
   */
  struct IRBundle {
    String name;
    //WDL_String name;
    int channelCount = 1;
    int sampleRate = 48000;
    size_t sampleCount = 0; // Samplecount for a single channel
    float** samples = nullptr; // Will always use floats since we only load from 16/24Bit wave 
    String path; // This is only set if it's a user IR
  };

  struct BlockSizeEvent {
    int blockSize;
    bool set = false;
  };
}