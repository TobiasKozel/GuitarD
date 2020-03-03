#pragma once
#include "../node/NodeSocket.h"
#include "../misc/MessageBus.h"
#include "../ui/theme.h"

namespace guitard {
  class NodeSocketUi : public IControl {
    MessageBus::Subscription<SocketConnectRequest> mOnConnectionEvent;
    MessageBus::Subscription<SocketConnectRequest> mOnConnectionRedirectEvent;
    MessageBus::Subscription<Node*> mOnDisconnectAllEvent;
    IMouseMod mMouseDown;

  protected:
    // NodeShared* mShared = nullptr;
    ConnectionDragData mDragData;
    NodeSocket* mSocket = nullptr;
    MessageBus::Bus* mBus = nullptr;
    IBlend mBlend;
    float mDiameter = 0;
    float mRadius = 0;
    int mIndex = -1;
    bool mOut = false;
  	
  public:
    NodeSocketUi(Coord2D parent, NodeSocket* socket, MessageBus::Bus* bus) : IControl({}, kNoParameter) {
      mBus = bus;
      mSocket = socket;
      mDiameter = Theme::Sockets::DIAMETER;
      mRadius = Theme::Sockets::DIAMETER * 0.5f;
      mBlend = EBlend::Default;
      mSocket->mAbs.x = mSocket->mRel.x + parent.x - mRadius;
      mSocket->mAbs.y = mSocket->mRel.y + parent.y - mRadius;
      mRECT.L = mSocket->mAbs.x;
      mRECT.T = mSocket->mAbs.y;
      mRECT.R = mRECT.L + mDiameter;
      mRECT.B = mRECT.T + mDiameter;
      SetTargetAndDrawRECTs(mRECT);

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

    void OnAttached() override {
      IRECT test = GetRECT();
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
      GetUI()->SetAllControlsDirty();
      IControl* target = GetUI()->GetControl(
        GetUI()->GetMouseControlIdx(x, y, false)
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
        SetRECT(GetUI()->GetBounds());
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
      GetUI()->SetAllControlsDirty();
    }
  };
}