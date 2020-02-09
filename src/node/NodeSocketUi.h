#pragma once
#include "../node/NodeSocket.h"
#include "../misc/MessageBus.h"
#include "../ui/theme.h"
#include "../node/NodeShared.h"

namespace guitard {
#ifndef GUITARD_HEADLESS
  class NodeSocketUi : public IControl {
    MessageBus::Subscription<SocketConnectRequest> mOnConnectionEvent;
    MessageBus::Subscription<SocketConnectRequest> mOnConnectionRedirectEvent;
    MessageBus::Subscription<Node*> mOnDisconnectAllEvent;
    IMouseMod mMouseDown;
  protected:
    NodeShared* mShared = nullptr;
    ConnectionDragData mDragData;
    IGraphics* mGraphics = nullptr;
    NodeSocket* mSocket = nullptr;
    MessageBus::Bus* mBus = nullptr;
    IBlend mBlend;
    float mDiameter = 0;
    float mRadius = 0;
    int mIndex = -1;
    bool mOut = false;
  public:
    NodeSocketUi(NodeShared* shared, NodeSocket* socket) :
      IControl(IRECT(0, 0, 0, 0), kNoParameter)
    {
      mShared = shared;
      mBus = shared->bus;
      mGraphics = shared->graphics;
      mSocket = socket;
      mDiameter = Theme::Sockets::DIAMETER;
      mRadius = Theme::Sockets::DIAMETER * 0.5f;
      mRECT.L = socket->mX;
      mRECT.T = socket->mY;
      mRECT.R = mRECT.L + mDiameter;
      mRECT.B = mRECT.T + mDiameter;
      SetTargetAndDrawRECTs(mRECT);
      mBlend = EBlend::Default;

      socket->callback = [&](bool doLock) {
        MessageBus::fireEvent(this->mBus, MessageBus::AwaitAudioMutex, doLock);
      };

      mOnConnectionEvent.subscribe(mBus, MessageBus::SocketConnect, [&](const SocketConnectRequest req) {
        if (req.to == this->mSocket) {
          this->mSocket->connect(req.from);
        }
      });

      mOnConnectionRedirectEvent.subscribe(mBus, MessageBus::SocketRedirectConnection, [&](const SocketConnectRequest req) {
        if (req.from == this->mSocket) {
          this->mSocket->connect(req.to);
        }
      });

      mOnDisconnectAllEvent.subscribe(mBus, MessageBus::NodeDisconnectAll, [&](Node* node) {
        if (this->mSocket->mParentNode == node) {
          this->mSocket->disconnectAll();
        }
      });

    }

    virtual ~NodeSocketUi() {
      mSocket->callback = nullptr;
    }

    void Draw(IGraphics& g) override {
      // This doesn't do the actual drawing since it needs to stay on top of the
      // layer stack. It's drawn in the cable layer and only a control to handle input
      if (mDragData.dragging) {
        // This is just a nice highlight around the socket beeing dragged
        g.FillCircle(Theme::Sockets::COLOR_ACTIVE, mDragData.startX,
          mDragData.startY, Theme::Sockets::ACTIVE_SIZE, &mBlend
        );
      }
    }


    void OnMouseDown(float x, float y, const IMouseMod& mod) override {
      mMouseDown = mod;
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
      if (mMouseDown.C) {
        mMouseDown = IMouseMod();
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
        else {
          MessageBus::fireEvent(mBus, MessageBus::QuickConnectSocket, QuickConnectRequest{
            Coord2D{x, y},
            mSocket
            });
        }
      }
      mMouseDown = IMouseMod();
    }

    void OnMouseDrag(const float x, const float y, float dX, float dY, const IMouseMod& mod) override {
      if (mMouseDown.C) { return; }
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
      mSocket->disconnectAll();
      mGraphics->SetAllControlsDirty();
    }
  };
#else
  class NodeSocketUi;
#endif
}