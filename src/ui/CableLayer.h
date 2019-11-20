#pragma once
#include "IControl.h"
#include "src/misc/constants.h"
#include "src/node/Node.h"
#include "src/node/NodeSocket.h"
#include "src/misc/MessageBus.h"
#include "src/misc/GStructs.h"

using namespace iplug;
using namespace igraphics;

/**
 * This control is a non clickable overlay over the whole plugin window to draw all the node connections
 * It also handles the splice in logic
 */
class CableLayer : public IControl {
  IGraphics* mGraphics;
  WDL_PtrList<Node>* mNodes;
  Node* mOutNode;
  Node* mInNode;
  NodeSocket* mHighlightSocket;
  IBlend mBlend;

  MessageBus::Bus* mBus;
  MessageBus::Subscription<Node*> mDisconnectAllEvent;
  MessageBus::Subscription<Coord2D> mNodeDraggedEvent;
  MessageBus::Subscription<Node*> mNodeDraggedEndEvent;
  MessageBus::Subscription<NodeSocket*> mPreviewSocketEvent;
  MessageBus::Subscription<SocketConnectRequest> onConnectionEvent;
  MessageBus::Subscription<Node*> mNodeDeleteEvent;
  MessageBus::Subscription<ConnectionDragData*> mConnectionDragEvent;

  NodeSocket* mPreviewSocketPrev;
  NodeSocket* mPreviewSocket;

  ConnectionDragData* mConnectionDragData;
public:
  CableLayer(MessageBus::Bus* pBus, IGraphics* g, WDL_PtrList<Node>* pNodes, Node* pOutNode, Node* pInNode) :
    IControl(IRECT(0, 0, g->Width(), g->Height()), kNoParameter)
  {
    mBus = pBus;
    mPreviewSocket = nullptr;
    mPreviewSocketPrev = nullptr;
    mHighlightSocket = nullptr;
    mConnectionDragData = nullptr;
    SetTargetRECT(IRECT(0, 0, 0, 0));
    mNodes = pNodes;
    mOutNode = pOutNode;
    mInNode = pInNode;
    mGraphics = g;
    mBlend = EBlend::Clobber;
    
    mDisconnectAllEvent.subscribe(mBus, MessageBus::NodeDisconnectAll, [&](Node*) {
      this->mDirty = true;
    });

    mNodeDraggedEvent.subscribe(mBus, MessageBus::NodeDragged, [&](Coord2D pos) {
      const float socketRadius = Theme::Sockets::DIAMETER / 2;
      mHighlightSocket = nullptr;
      Node* curNode;
      NodeSocket* curSock;
      NodeSocket* tarSock;
      for (int n = 0; n < mNodes->GetSize() + 1; n++) {
        curNode = mNodes->Get(n);
        if (curNode == nullptr) {
          // only happens for the last node
          curNode = mOutNode;
        }
        for (int i = 0; i < curNode->mInputCount; i++) {
          curSock = curNode->mSocketsIn.Get(i);
          tarSock = curSock->mConnectedTo;
          if (tarSock != nullptr) {
            float x1 = tarSock->mX + socketRadius;
            float x2 = curSock->mX + socketRadius;
            float y1 = tarSock->mY + socketRadius;
            float y2 = curSock->mY + socketRadius;
            IRECT box;
            box.L = min(x1, x2) - SPLICEIN_DISTANCE;
            box.R = max(x1, x2) + SPLICEIN_DISTANCE;
            box.T = min(y1, y2) - SPLICEIN_DISTANCE;
            box.B = max(y1, y2) + SPLICEIN_DISTANCE;
            if (box.Contains(IRECT{ pos.x, pos.y, pos.x, pos.y })) {
              const float a = y1 - y2;
              const float b = x2 - x1;
              const float c = x1 * y2 - x2 * y1;
              const float d = abs(a * pos.x + b * pos.y + c) / sqrt(a * a + b * b);
              if (d < SPLICEIN_DISTANCE) {
                mHighlightSocket = curSock;
                break;
              }
            }
          }
        }
      }
    });

    mNodeDraggedEndEvent.subscribe(mBus, MessageBus::NodeDraggedEnd, [&](Node* node) {
      NodeSocket* target = mHighlightSocket;
      mHighlightSocket = nullptr;
      mDirty = true;
      if (target != nullptr) {
        if (target->mParentNode == node) {
          return;
        }
        Node* targetNode = target->mParentNode;
        for (int i = 0; i < targetNode->mInputCount; i++) {
          if (targetNode->mSocketsIn.Get(i)->mConnectedTo != nullptr &&
              targetNode->mSocketsIn.Get(i)->mConnectedTo->mParentNode == node) {
            return;
          }
        }
        MessageBus::fireEvent<NodeSpliceInPair>(this->mBus, MessageBus::NodeSpliceIn, NodeSpliceInPair{ node, target });
      }
    });

    mPreviewSocketEvent.subscribe(mBus, MessageBus::PreviewSocket, [&](NodeSocket* socket) {
      // TODO this is kinda shady and does not use the MessageBus::SocketConnect event
      NodeSocket* outSocket = this->mOutNode->mSocketsIn.Get(0);
      if (outSocket->mConnectedTo == socket) { return; }
      if (socket == this->mPreviewSocket) {
        // Connect the original socket again
        if (this->mPreviewSocketPrev != nullptr) {
          outSocket->connect(this->mPreviewSocketPrev);
          this->mPreviewSocketPrev = nullptr;
        }
        this->mPreviewSocket = nullptr;
      }
      else {
        // Save the currently connected socket and connect it to the one provided
        this->mPreviewSocketPrev = outSocket->mConnectedTo;
        this->mPreviewSocket = socket;
        outSocket->connect(socket);
      }
      this->mDirty = true;
    });

    onConnectionEvent.subscribe(mBus, MessageBus::SocketConnect, [&](SocketConnectRequest req) {
      mPreviewSocket = nullptr;
      mPreviewSocketPrev = nullptr;
    });

    mNodeDeleteEvent.subscribe(mBus, MessageBus::NodeDeleted, [&](Node* param) {
      this->mPreviewSocketPrev = nullptr;
      this->mPreviewSocket = nullptr;
      this->mDirty = true;
    });

    mConnectionDragEvent.subscribe(mBus, MessageBus::ConnectionDragged, [&](ConnectionDragData* d) {
      if (d->dragging) {
        this->mConnectionDragData = d;
        this->mDirty = true;
      }
      else {
        this->mConnectionDragData = nullptr;
      }
    });
  }

  inline void DrawSocket(IGraphics& g, NodeSocket* s) {
    float x = s->mX + Theme::Sockets::RADIUS;
    float y = s->mY + Theme::Sockets::RADIUS;
    g.FillCircle(
      Theme::Sockets::COLOR, x, y,
      Theme::Sockets::RADIUS * 0.5 * Theme::Sockets::OUTLINE_SIZE, &mBlend
    );
    g.FillCircle(
      Theme::Sockets::COLOR_INNER, x, y,
      Theme::Sockets::RADIUS * 0.4, &mBlend
    );
  }

  void Draw(IGraphics& g) override {
    Node* curNode;
    NodeSocket* curSock;
    NodeSocket* tarSock;
    const float socketRadius = Theme::Sockets::DIAMETER / 2;
    // Draw all the connections between nodes
    for (int n = 0; n < mNodes->GetSize() + 1; n++) {
      curNode = mNodes->Get(n);
      if (curNode == nullptr) {
        // only happens for the last node
        curNode = mOutNode;
      }
      for (int i = 0; i < curNode->mInputCount; i++) {
        curSock = curNode->mSocketsIn.Get(i);
        if (curSock->mConnectedTo != nullptr) {
          tarSock = curSock->mConnectedTo;
          if (tarSock == mPreviewSocket && curSock == mOutNode->mSocketsIn.Get(0)) {
            // Draw the temporary bypass
            g.DrawDottedLine(
              curSock == mHighlightSocket ? Theme::Cables::COLOR_SPLICE_IN : Theme::Cables::COLOR,
              curSock->mX + socketRadius, curSock->mY + socketRadius,
              tarSock->mX + socketRadius, tarSock->mY + socketRadius,
              &mBlend, Theme::Cables::THICKNESS, Theme::Cables::PREVIEW_DASH_DIST
            );
            if (mPreviewSocketPrev != nullptr) {
              // draw the original connection slightly transparent
              g.DrawLine(
                curSock == mHighlightSocket ? Theme::Cables::COLOR_SPLICE_IN : Theme::Cables::COLOR_PREVIEW,
                curSock->mX + socketRadius, curSock->mY + socketRadius,
                mPreviewSocketPrev->mX + socketRadius, mPreviewSocketPrev->mY + socketRadius,
                &mBlend, Theme::Cables::THICKNESS
              );
            }
          }
          else {
            g.DrawLine(
              curSock == mHighlightSocket ? Theme::Cables::COLOR_SPLICE_IN : Theme::Cables::COLOR,
              curSock->mX + socketRadius, curSock->mY + socketRadius,
              tarSock->mX + socketRadius, tarSock->mY + socketRadius,
              &mBlend, Theme::Cables::THICKNESS
            );
          }
        }
      }
    }

    // Draw a new connection from the user to start socket;
    if (mConnectionDragData != nullptr) {
      g.DrawLine(
        Theme::Cables::COLOR, mConnectionDragData->startX, mConnectionDragData->startY,
        mConnectionDragData->currentX, mConnectionDragData->currentY, &mBlend, Theme::Cables::THICKNESS
      );
    }

    // Draw all the sockets
    for (int n = 0; n < mNodes->GetSize(); n++) {
      curNode = mNodes->Get(n);
      for (int i = 0; i < curNode->mOutputCount; i++) {
        curSock = curNode->mSocketsOut.Get(i);
        if (curSock != nullptr) {
          DrawSocket(g, curSock);
        }
      }
      for (int i = 0; i < curNode->mInputCount; i++) {
        curSock = curNode->mSocketsIn.Get(i);
        if (curSock != nullptr) {
          DrawSocket(g, curSock);
        }
      }
    }
    DrawSocket(g, mOutNode->mSocketsIn.Get(0));
    DrawSocket(g, mInNode->mSocketsOut.Get(0));


    
    // g.FillRect(COLOR_GRAY, mRECT);
  }

  void OnResize() override {
    mRECT.R = mGraphics->Width();
    mRECT.B = mGraphics->Height();
  }

};