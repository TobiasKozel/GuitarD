#pragma once
#include "IControl.h"
#include "src/misc/constants.h"
#include "src/node/NodeSocket.h"
#include "src/misc/MessageBus.h"
#include "src/ui/theme.h"

using namespace iplug;
using namespace igraphics;

class NodeSocketUi : public IControl {
  MessageBus::Subscription<SocketConnectRequest> onConnectionEvent;
  MessageBus::Subscription<SocketConnectRequest> onConnectionRedirectEvent;
  MessageBus::Subscription<Node*> onDisconnectAllEvent;
  MessageBus::Subscription<NodeSocket*> onDisconnectEvent;
  int vol;
  IMouseMod mMousDown;
public:
  NodeSocketUi(MessageBus::Bus* pBus, IGraphics* g, NodeSocket* socket, float x, float y) :
    IControl(IRECT(0, 0, 0, 0), kNoParameter)
  {
    mBus = pBus;
    mSocket = socket;
    mDiameter = SOCKETDIAMETER;
    mRadius = SOCKETDIAMETER * 0.5f;
    mRECT.L = x;
    mRECT.T = y;
    mRECT.R = mRECT.L + mDiameter;
    mRECT.B = mRECT.T + mDiameter;
    SetTargetAndDrawRECTs(mRECT);
    mBlend = EBlend::Clobber;
    mGraphics = g;

    mSocket->X = mTargetRECT.L;
    mSocket->Y = mTargetRECT.T;

    onConnectionEvent.subscribe(mBus, MessageBus::SocketConnect, [&](SocketConnectRequest req) {
      if (req.to == this->mSocket) {
        this->mSocket->connect(req.from);
      }
    });

    onConnectionRedirectEvent.subscribe(mBus, MessageBus::SocketRedirectConnection, [&](SocketConnectRequest req) {
      if (req.from == this->mSocket->connectedTo) {
        this->mSocket->connect(req.to);
      }
    });

    onDisconnectAllEvent.subscribe(mBus, MessageBus::NodeDisconnectAll, [&](Node* node) {
      if (this->mSocket->parentNode == node) {
        mSocket->disconnect();
      }
    });

    onDisconnectEvent.subscribe(mBus, MessageBus::DisconnectSocket, [&](NodeSocket * socket) {
      if (this->mSocket->connectedTo == socket && this->mSocket->isInput) {
        this->mSocket->disconnect();
      }
    });
  }

  void Draw(IGraphics& g) override {
    // This doesn't do the actual drawing since it needs to stay on top of the
    // layer stack. It's drawn in the cable layer and only a control to handle input
    if (mDragData.dragging) {
      // This is just a nice highlight around the socket beeing dragged
      g.FillCircle(SOCKETCOLORACITVE, mDragData.startX,
        mDragData.startY, SOCKETATIVESIZE, &mBlend
      );
    }
  }


  void OnMouseDown(float x, float y, const IMouseMod& mod) override {
    mMousDown = mod;
    if (mod.C) {
      MessageBus::fireEvent<NodeSocket*>(mBus, MessageBus::PreviewSocket, mSocket);
    }
    else {
      auto center = mRECT;
      center.ScaleAboutCentre(0);
      mDragData.startX = center.L;
      mDragData.startY = center.T;
    }
  }

  virtual void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    mDragData.dragging = false;
    MessageBus::fireEvent<ConnectionDragData*>(
      mBus, MessageBus::ConnectionDragged, &mDragData
    );
    if (mMousDown.C) {
      mMousDown = IMouseMod();
      return;
    }

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
          mBus,
          MessageBus::SocketConnect,
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
    if (mMousDown.C) { return; }
    if (!mDragData.dragging) {
      SetRECT(mGraphics->GetBounds());
      mDragData.dragging = true;
    }
    mDragData.currentX = x;
    mDragData.currentY = y;
    MessageBus::fireEvent<ConnectionDragData*>(
      mBus, MessageBus::ConnectionDragged, &mDragData
    );
  }

  /**
   * Disconnect the input on double click
   */
  void OnMouseDblClick(float x, float y, const IMouseMod& mod) override {
    mSocket->disconnect();
    mGraphics->SetAllControlsDirty();
  }

protected:
  ConnectionDragData mDragData;
  MessageBus::Bus* mBus;
  NodeSocket* mSocket;
  IBlend mBlend;
  IGraphics* mGraphics;
  float mDiameter;
  float mRadius;
  int mIndex;
  bool mOut;
};
