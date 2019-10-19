#pragma once
#include "IControl.h"
#include "src/constants.h"
#include "src/graph/Node.h"
#include "src/graph/misc/NodeSocket.h"

using namespace iplug;
using namespace igraphics;

class CableLayer : public IControl {
public:
  CableLayer(IGraphics* g, WDL_PtrList<Node>* pNodes) :
    IControl(IRECT(0, 0, g->Width(), g->Height()), kNoParameter)
  {
    SetTargetRECT(IRECT(0, 0, 0, 0));
    mNodes = pNodes;
    mGraphics = g;
    mBlend = EBlend::Clobber;
    mColor.A = 255;
    mColor.R = 255;
  }

  void Draw(IGraphics& g) override {
    Node* curNode;
    NodeSocket* curSock;
    NodeSocket* tarSock;
    float socketRadius = SOCKETDIAMETER / 2;
    for (int n = 0; n < mNodes->GetSize(); n++) {
      curNode = mNodes->Get(n);
      for (int i = 0; i < curNode->inputCount; i++) {
        curSock = curNode->inSockets.Get(i);
        if (curSock->connectedNode != nullptr) {
          tarSock = curSock->connectedNode->outSockets.Get(curSock->connectedBufferIndex);
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
  IColor mColor;
  IBlend mBlend;
};