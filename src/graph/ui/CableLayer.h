#pragma once
#include "IControl.h"
#include "src/constants.h"
#include "src/graph/Node.h"
#include "src/graph/misc/NodeSocket.h"
#include "src/graph/misc/MessageBus.h"

using namespace iplug;
using namespace igraphics;

class CableLayer : public IControl {
  MessageBus::Subscription<Node*> mDisconnectAllEvent;
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
    
    mDisconnectAllEvent.subscribe("NodeDisconnectAll", [&](Node*) {
      this->mDirty = true;
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
        curNode = mOutNode;
      }
      for (int i = 0; i < curNode->inputCount; i++) {
        curSock = curNode->inSockets.Get(i);
        if (curSock->connectedTo != nullptr) {
          tarSock = curSock->connectedTo;
          g.DrawLine(
            mColor,
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
  IColor mColor;
  IBlend mBlend;
};