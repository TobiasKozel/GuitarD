#pragma once
#include "IControl.h"
#include "src/misc/constants.h"
#include "src/node/Node.h"
#include "src/node/NodeSocket.h"
#include "src/misc/MessageBus.h"
#include "src/misc/GStructs.h"

using namespace iplug;
using namespace igraphics;

class CableLayer : public IControl {

  MessageBus::Subscription<Node*> mDisconnectAllEvent;
  MessageBus::Subscription<Coord2d> mNodeDraggedEvent;
  MessageBus::Subscription<Node*> mNodeDraggedEndEvent;
  MessageBus::Subscription<NodeSocket*> mPreviewSocketEvent;
  MessageBus::Subscription<SocketConnectRequest> onConnectionEvent;

  NodeSocket* mPreviewSocketPrev;
  NodeSocket* mPreviewSocket;
public:
  CableLayer(IGraphics* g, WDL_PtrList<Node>* pNodes, Node* pOutNode) :
    IControl(IRECT(0, 0, g->Width(), g->Height()), kNoParameter)
  {
    mPreviewSocket = nullptr;
    mPreviewSocketPrev = nullptr;
    SetTargetRECT(IRECT(0, 0, 0, 0));
    mNodes = pNodes;
    mOutNode = pOutNode;
    mGraphics = g;
    mBlend = EBlend::Clobber;
    mColor.A = 255;
    mColor.R = 255;

    mColorHighlight.A = 255;
    mColorHighlight.G = 255;

    mColorPreview.A = 100;
    mColorPreview.R = 255;

    mHighlightSocket = nullptr;
    
    mDisconnectAllEvent.subscribe("NodeDisconnectAll", [&](Node*) {
      this->mDirty = true;
    });

    mNodeDraggedEvent.subscribe("NodeDragged", [&](Coord2d pos) {
      float socketRadius = SOCKETDIAMETER / 2;
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
        for (int i = 0; i < curNode->inputCount; i++) {
          curSock = curNode->inSockets.Get(i);
          if (curSock->connectedTo != nullptr) {
            tarSock = curSock->connectedTo;
            IRECT box;
            box.L = min(tarSock->X, curSock->X) + socketRadius;
            box.R = max(tarSock->X, curSock->X) + socketRadius;
            box.T = min(tarSock->Y, curSock->Y) + socketRadius;
            box.B = max(tarSock->Y, curSock->Y) + socketRadius;
            if (box.Contains(IRECT{ pos.x, pos.y, pos.x, pos.y })) {
              float a = box.T - box.B;
              float b = box.R - box.L;
              float c = box.L * box.B - box.R * box.T;
              float d = abs(a * pos.x + b * pos.y + c) / sqrt(a * a + b * b);
              if (d < 15) {
                mHighlightSocket = curSock;
                break;
              }
            }
          }
        }
      }
    });

    mNodeDraggedEndEvent.subscribe("NodeDraggedEnd", [&](Node* node) {
      if (mHighlightSocket != nullptr) {
        MessageBus::fireEvent<NodeSpliceInPair>("NodeSpliceIn", NodeSpliceInPair{ node, mHighlightSocket });
      }
      mHighlightSocket = nullptr;
      mDirty = true;
    });

    mPreviewSocketEvent.subscribe("PreviewSocket", [&](NodeSocket* socket) {
      // TODO this is kinda shady and does not use the "SocketConnect" event
      NodeSocket* outSocket = this->mOutNode->inSockets.Get(0);
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
        this->mPreviewSocketPrev = outSocket->connectedTo;
        this->mPreviewSocket = socket;
        outSocket->connect(socket);
      }
      this->mDirty = true;
    });

    onConnectionEvent.subscribe("SocketConnect", [&](SocketConnectRequest req) {
      mPreviewSocket = nullptr;
      mPreviewSocketPrev = nullptr;
    });
  }

  void Draw(IGraphics& g) override {
    Node* curNode;
    NodeSocket* curSock;
    NodeSocket* tarSock;
    float socketRadius = SOCKETDIAMETER / 2;
    for (int n = 0; n < mNodes->GetSize() + 1; n++) {
      curNode = mNodes->Get(n);
      if (curNode == nullptr) {
        // only happens for the last node
        curNode = mOutNode;
      }
      for (int i = 0; i < curNode->inputCount; i++) {
        curSock = curNode->inSockets.Get(i);
        if (curSock->connectedTo != nullptr) {
          tarSock = curSock->connectedTo;
          if (tarSock == mPreviewSocket && curSock == mOutNode->inSockets.Get(0)) {
            // Draw the temporary bypass
            g.DrawDottedLine(
              curSock == mHighlightSocket ? mColorHighlight : mColor,
              curSock->X + socketRadius, curSock->Y + socketRadius,
              tarSock->X + socketRadius, tarSock->Y + socketRadius,
              &mBlend, 5, 20
            );
            if (mPreviewSocketPrev != nullptr) {
              // draw the original connection slightly transparent
              g.DrawLine(
                curSock == mHighlightSocket ? mColorHighlight : mColorPreview,
                curSock->X + socketRadius, curSock->Y + socketRadius,
                mPreviewSocketPrev->X + socketRadius, mPreviewSocketPrev->Y + socketRadius,
                &mBlend, 5
              );
            }
          }
          else {
            g.DrawLine(
              curSock == mHighlightSocket ? mColorHighlight : mColor,
              curSock->X + socketRadius, curSock->Y + socketRadius,
              tarSock->X + socketRadius, tarSock->Y + socketRadius,
              &mBlend, 5
            );
          }
        }
      }
    }
    // g.FillRect(COLOR_GRAY, mRECT);
  }

  void OnResize() override {
    mRECT.R = mGraphics->Width();
    mRECT.B = mGraphics->Height();
  }

private:
  IGraphics* mGraphics;
  WDL_PtrList<Node>* mNodes;
  Node* mOutNode;
  NodeSocket* mHighlightSocket;
  IColor mColor;
  IColor mColorPreview;
  IColor mColorHighlight;
  IBlend mBlend;
};