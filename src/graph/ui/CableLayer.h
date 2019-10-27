#pragma once
#include "IControl.h"
#include "src/constants.h"
#include "src/graph/Node.h"
#include "src/graph/misc/NodeSocket.h"
#include "src/graph/misc/MessageBus.h"
#include "src/graph/misc/GStructs.h"

using namespace iplug;
using namespace igraphics;

class CableLayer : public IControl {
  MessageBus::Subscription<Node*> mDisconnectAllEvent;
  MessageBus::Subscription<Coord2d> mNodeDraggedEvent;
  MessageBus::Subscription<Coord2d> mNodeDraggedEndEvent;
public:
  CableLayer(IGraphics* g, WDL_PtrList<Node>* pNodes, Node* pOutNode) :
    IControl(IRECT(0, 0, g->Width(), g->Height()), kNoParameter)
  {
    SetTargetRECT(IRECT(0, 0, 0, 0));
    mNodes = pNodes;
    mOutNode = pOutNode;
    mGraphics = g;
    mBlend = EBlend::Clobber;
    mColor.A = 255;
    mColor.R = 255;

    mColorHighlight.A = 255;
    mColorHighlight.G = 255;

    mHighlightSocket = nullptr;
    
    mDisconnectAllEvent.subscribe("NodeDisconnectAll", [&](Node*) {
      this->mDirty = true;
    });

    mNodeDraggedEvent.subscribe("NodeDragged", [&](Coord2d pos) {
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
            box.L = min(tarSock->X, curSock->X);
            box.R = max(tarSock->X, curSock->X);
            box.T = min(tarSock->Y, curSock->Y);
            box.B = max(tarSock->Y, curSock->Y);
            if (box.Contains(IRECT{ pos.x, pos.y, pos.x, pos.y })) {
              mHighlightSocket = curSock;
            }
          }
        }
      }
    });

    mNodeDraggedEndEvent.subscribe("NodeDraggedEnd", [&](Coord2d pos) {
      if (mHighlightSocket != nullptr) {
        MessageBus::fireEvent<NodeSocket*>("NodeSpliceIn", mHighlightSocket);
      }
      mHighlightSocket = nullptr;
      mDirty = true;
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
          g.DrawLine(
            curSock == mHighlightSocket ? mColorHighlight : mColor,
            curSock->X + socketRadius, curSock->Y + socketRadius,
            tarSock->X + socketRadius, tarSock->Y + socketRadius,
            &mBlend, 5
          );
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
  IColor mColorHighlight;
  IBlend mBlend;
};