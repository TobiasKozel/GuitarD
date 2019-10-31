#pragma once
#include "IControl.h"
#include "src/misc/constants.h"
#include "src/node/NodeSocket.h"
#include "src/misc/MessageBus.h"

using namespace iplug;
using namespace igraphics;

struct SocketConnectRequest {
  NodeSocket* from;
  NodeSocket* to;
};

class NodeSocketUi : public IControl {
  MessageBus::Subscription<SocketConnectRequest> onConnectionEvent;
  MessageBus::Subscription<SocketConnectRequest> onConnectionRedirectEvent;
  MessageBus::Subscription<Node*> onDisconnectAllEvent;
  MessageBus::Subscription<NodeSocket*> onDisconnectEvent;
  int vol;
  IMouseMod mMousDown;
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

    onConnectionEvent.subscribe("SocketConnect", [&](SocketConnectRequest req) {
      if (req.to == this->mSocket) {
        this->mSocket->connect(req.from);
      }
    });

    onConnectionRedirectEvent.subscribe("SocketRedirectConnection", [&](SocketConnectRequest req) {
      if (req.from == this->mSocket->connectedTo) {
        this->mSocket->connect(req.to);
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
    mMousDown = mod;
    if (mod.C) {
      MessageBus::fireEvent<NodeSocket*>("PreviewSocket", mSocket);
    }
    else {
      // HACK
      // this will mean the control is attached twice once on top of the drawing stack
      mGraphics->AttachControl(this);
      auto center = mRECT;
      center.ScaleAboutCentre(0);
      mStartX = center.L;
      mStartY = center.T;
    }
  }

  virtual void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    if (mMousDown.C) {
      mMousDown = IMouseMod();
      return;
    }
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
          "SocketConnect",
          SocketConnectRequest{
            mSocket,
            targetSocket
          }
        );
      }
    }
    mMousDown = IMouseMod();
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