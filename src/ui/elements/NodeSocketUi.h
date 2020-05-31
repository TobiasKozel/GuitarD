#pragma once
#include "../../main/NodeSocket.h"
#include "../bus/MessageBus.h"
#include "../GUIConfig.h"

namespace guitard {
  class NodeSocketUi : public IControl {
    IMouseMod mMouseDown;

  protected:
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
    }

    void Draw(IGraphics& g) override {
      // This doesn't do the actual drawing since it needs to stay on top of the
      // layer stack. It's drawn in the cable layer and only a control to handle input
      if (mDragData.dragging) {
        // This is just a nice highlight around the socket being dragged
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

    void OnMouseUp(float x, float y, const IMouseMod& mod) override {
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
          if (targetSocket != mSocket) { // don't try connecting to itself
            MessageBus::fireEvent<SocketConnectRequest>(
              mBus, MessageBus::SocketConnect, { mSocket, targetSocket }
            );
          }
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
      MessageBus::fireEvent<SocketConnectRequest>(
        mBus, MessageBus::SocketConnect, { mSocket, nullptr }
      );
      GetUI()->SetAllControlsDirty();
    }
  };
}