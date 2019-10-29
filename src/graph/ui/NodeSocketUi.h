#pragma once
#include "IControl.h"
#include "src/constants.h"
#include "src/graph/misc/NodeSocket.h"
#include "src/graph/misc/MessageBus.h"

using namespace iplug;
using namespace igraphics;

struct SocketConnectRequest {
  NodeSocket* from;
  NodeSocket* to;
};

class NodeSocketUi : public IControl {
  MessageBus::Subscription<SocketConnectRequest> onConnectionEvent;
  MessageBus::Subscription<Node*> onDisconnectAllEvent;
  MessageBus::Subscription<NodeSocket*> onDisconnectEvent;
  int vol;
public:
  NodeSocketUi(IGraphics* g, NodeSocket* socket) :
    IControl(IRECT(0, 0, 0, 0), kNoParameter)
  {
    //mBitmap = g->LoadBitmap(bitmap, 1, false);
    //mRECT.R = L + mBitmap.W();
    //mRECT.B = T + mBitmap.H();
    mSocket = socket;
    mDiameter = SOCKETDIAMETER;
    mRadius = mDiameter * 0.5f;
    mRECT.L = socket->X;
    mRECT.T = socket->Y;
    mRECT.R = mRECT.L + mDiameter;
    mRECT.B = mRECT.T + mDiameter;
    SetTargetAndDrawRECTs(mRECT);
    mBlend = EBlend::Clobber;
    mGraphics = g;
    color.A = 255;
    if (socket->isInput) {
      color.R = 255;
    }
    else {
      color.B = 255;
    }
    onConnectionEvent.subscribe("socketConnection", [&](SocketConnectRequest req) {
      if (req.to == this->mSocket) {
        this->mSocket->connect(req.from);
      }
    });

    onDisconnectAllEvent.subscribe("NodeDisconnectAll", [&](Node* node) {
      if (this->mSocket->parentNode == node) {
        mSocket->disconnect();
      }
    });

    onDisconnectEvent.subscribe("DisconnectSocket", [&](NodeSocket * socket) {
      if (this->mSocket->connectedTo == socket && this->mSocket->isInput) {
        this->mSocket->disconnect();
      }
    });
  }

  void Draw(IGraphics& g) override {
    // g.DrawBitmap(mBitmap, GetRECT(), 1, &mBlend);
    //double avg = 0;
    //if (mSocket->buffer != nullptr) {
    //  for (int i = 0; i < 64; i++) {
    //    avg += abs(mSocket->buffer[0][i]);
    //  }
    //}
    //vol = SkClampMax(avg * 50, 255);
    //g.DrawCircle(IColor(255, vol, 0, 0), mTargetRECT.L + mRadius, mTargetRECT.T + mRadius, 4, &mBlend, 10);
    //mDirty = true;
    g.DrawCircle(color, mTargetRECT.L + mRadius, mTargetRECT.T + mRadius, mRadius, &mBlend, 10);
    if (mDragging) {
      g.DrawLine(color, mStartX, mStartY, mCurrentX, mCurrentY, &mBlend, 5);
    }
  }


  void OnMouseDown(float x, float y, const IMouseMod& mod) override {
    // HACK
    // this will mean the control is attached twice once on toop of the drawing stack
    mGraphics->AttachControl(this);
    auto center = mRECT;
    center.ScaleAboutCentre(0);
    mStartX = center.L;
    mStartY = center.T;
  }

  virtual void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    // this will get rid of the top most duplicate
    mGraphics->RemoveControl(this);
    mDragging = false;
    SetRECT(mTargetRECT);
    mGraphics->SetAllControlsDirty();
    IControl* target = mGraphics->GetControl(
      mGraphics->GetMouseControlIdx(x, y, false)
    );
    if (target != nullptr) {
      NodeSocketUi* targetUi = dynamic_cast<NodeSocketUi*>(target);
      if (targetUi != nullptr) {
        NodeSocket* targetSocket = targetUi->mSocket;
        MessageBus::fireEvent<SocketConnectRequest>(
          "socketConnection",
          SocketConnectRequest{
            mSocket,
            targetSocket
          }
        );
      }
    }
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    mDragging = true;
    SetRECT(IRECT(0, 0, 2000, 2000));
    mCurrentX = x;
    mCurrentY = y;
    mGraphics->SetAllControlsDirty();
  }

  /**
   * Disconnect the input on double click
   */
  void OnMouseDblClick(float x, float y, const IMouseMod& mod) override {
    mSocket->disconnect();
    mGraphics->SetAllControlsDirty();
  }

protected:
  NodeSocket* mSocket;
  IBitmap mBitmap;
  IBlend mBlend;
  IGraphics* mGraphics;
  IColor color;
  float mDiameter;
  float mRadius;
  int mIndex;
  bool mOut;
  bool mDragging;
  float mStartX;
  float mStartY;
  float mCurrentY;
  float mCurrentX;

};