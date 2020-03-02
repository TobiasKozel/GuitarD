#pragma once
#ifndef GUITARD_HEADLESS

#include "IControl.h"
#include "../misc/constants.h"
#include "../node/Node.h"
#include "../node/NodeSocket.h"
#include "../misc/MessageBus.h"
#include "../types/gstructs.h"

namespace guitard {
  /**
   * Oh boy, this ones a burning mess I probably should be sorry for
   * This control is a non clickable overlay over the whole plugin window to draw all the node connections
   * It also handles the splice in logic
   */
  class CableLayer : public IControl {
    PointerList<NodeUi>* mNodes; // Pointer to all the node UIs
    Node* mOutNode = nullptr; // Out node needs some special treatment for quick preview
    Node* mInNode = nullptr;

    Node* mVisualizeAutomation = nullptr; // The node to visualize all the automation targets for

    Node* mPickAutomationTarget = nullptr; // The node for which the target picker mode is activated

    IBlend mBlend; // TODO might not be needed

    MessageBus::Bus* mBus = nullptr;
    MessageBus::Subscription<Node*> mDisconnectAllEvent;
    MessageBus::Subscription<Drag> mNodeDraggedEvent;
    MessageBus::Subscription<Coord2D> mNodeSeverEvent;
    MessageBus::Subscription<NodeDragEndData> mNodeDraggedEndEvent;
    MessageBus::Subscription<NodeSocket*> mPreviewSocketEvent;
    MessageBus::Subscription<SocketConnectRequest> onConnectionEvent;
    MessageBus::Subscription<Node*> mNodeDeleteEvent;
    MessageBus::Subscription<ConnectionDragData*> mConnectionDragEvent;
    MessageBus::Subscription<Node*> mVisualizeAutomationTargetsEvent;
    MessageBus::Subscription<Node*> mPickAutomationTargetEvent;
    MessageBus::Subscription<GraphStats*> mGraphInvalidEvent;

    // Used to highlight the connection just before a splice in
    NodeSocket* mHighlightSocket = nullptr;
    // Used to indicate the previous connection when doing quick previews
    NodeSocket* mPreviewSocketPrev = nullptr;
    // The temporary preview connection
    NodeSocket* mPreviewSocket = nullptr;

    ConnectionDragData* mConnectionDragData = nullptr;

    GraphStats* mStats = nullptr;
  public:
    CableLayer(MessageBus::Bus* pBus, PointerList<NodeUi>* pNodes) :
        IControl(IRECT(), kNoParameter)
    {
      mBus = pBus;
      SetTargetRECT(IRECT(0, 0, 0, 0));
      mNodes = pNodes;
      mBlend = EBlend::Default;

      mDisconnectAllEvent.subscribe(mBus, MessageBus::NodeDisconnectAll, [&](Node*) {
        mDirty = true;
      });

      mNodeDraggedEvent.subscribe(mBus, MessageBus::NodeDragged, [&](const Drag drag) {
        mHighlightSocket = getClosestToConnection(drag.pos);
      });

      mNodeSeverEvent.subscribe(mBus, MessageBus::SeverNodeConnection, [&](const Coord2D pos) {
        NodeSocket* close = getClosestToConnection(pos);
        if (close != nullptr) {
          close->disconnectAll();
        }
        mDirty = true;
      });

      mNodeDraggedEndEvent.subscribe(mBus, MessageBus::NodeDraggedEnd, [&](const NodeDragEndData data) {
        Node* node = data.node;
        NodeSocket* target = mHighlightSocket;
        mHighlightSocket = nullptr;
        mDirty = true;
        if (node->mInputCount == 0 || node->mOutputCount == 0) { return; }
        if (target != nullptr) {
          if (target->mParentNode == node) {
            return;
          }
          Node* targetNode = target->mParentNode;
          for (int i = 0; i < targetNode->mInputCount; i++) {
            if (targetNode->mSocketsIn[i]->mConnectedTo[0] != nullptr &&
              targetNode->mSocketsIn[i]->mConnectedTo[0]->mParentNode == node) {
              return;
            }
          }
          MessageBus::fireEvent<NodeSpliceInPair>(this->mBus, MessageBus::NodeSpliceIn, NodeSpliceInPair{ node, target });
          const float distance = abs(node->mPos.x - targetNode->mPos.x);
          if (distance < (node->mDimensions.x + 80)) {
            // targetNode->moveAlong(node->shared.width - distance + 200); // TODO reenable this once move along works again
          }
        }
      });

      mPreviewSocketEvent.subscribe(mBus, MessageBus::PreviewSocket, [&](NodeSocket* socket) {
        // WDBGMSG(socket->mParentNode->mType.c_str());
        // TODOG this is kinda shady and does not use the MessageBus::SocketConnect event
        if (mOutNode == nullptr) { return; }
        NodeSocket* outSocket = mOutNode->mSocketsIn[0];
        if (socket == this->mPreviewSocketPrev || socket == this->mPreviewSocket) {
          // If the socket clicked is the current preview socket, connect the original socket again
          if (this->mPreviewSocketPrev != nullptr) {
            outSocket->connect(this->mPreviewSocketPrev);
            this->mPreviewSocketPrev = nullptr;
          }
          this->mPreviewSocket = nullptr;
        }
        else {
          // Don't do anything if the socket is already connected to the output node
          if (outSocket->mConnectedTo[0] == socket) { return; }
          // Save the currently connected socket and connect it to the one provided
          if (this->mPreviewSocket == nullptr) {
            this->mPreviewSocketPrev = outSocket->mConnectedTo[0];
          }
          this->mPreviewSocket = socket;
          outSocket->connect(socket);
        }
        this->mDirty = true;
      });

      onConnectionEvent.subscribe(mBus, MessageBus::SocketConnect, [&](SocketConnectRequest req) {
        // TODOG maybe connect back to the original instead of making the temp preview the active one
        mPreviewSocket = nullptr;
        mPreviewSocketPrev = nullptr;
      });

      mNodeDeleteEvent.subscribe(mBus, MessageBus::NodeDeleted, [&](Node* param) {
        this->mPreviewSocketPrev = nullptr;
        this->mPreviewSocket = nullptr;
        this->mDirty = true;
      });

      mConnectionDragEvent.subscribe(mBus, MessageBus::ConnectionDragged, [&](ConnectionDragData* d) {
        if (d->dragging) {
          this->mConnectionDragData = d;
          this->mDirty = true;
        }
        else {
          this->mConnectionDragData = nullptr;
        }
      });

      mVisualizeAutomationTargetsEvent.subscribe(mBus, MessageBus::VisualizeAutomationTargets, [&](Node* n) {
        this->mVisualizeAutomation = n;
        this->mDirty = true;
      });

      mPickAutomationTargetEvent.subscribe(mBus, MessageBus::PickAutomationTarget, [&](Node* n) {
        this->mPickAutomationTarget = n;
        SetTargetRECT(GetUI()->GetBounds());
        this->mDirty = true;
      });

      mGraphInvalidEvent.subscribe(mBus, MessageBus::GraphStatsChanged, [&](GraphStats* stats) {
        this->mStats = stats;
        this->mDirty = true;
      });
    }

    void setInOutNodes(Node* in, Node* out) {
      mInNode = in;
      mOutNode = out;
    }

    void DrawSocket(IGraphics& g, NodeSocket* s) const {
      const float x = s->mX + Theme::Sockets::RADIUS;
      const float y = s->mY + Theme::Sockets::RADIUS;
      g.FillCircle(
        Theme::Sockets::COLOR, x, y,
        Theme::Sockets::RADIUS * 0.5 * Theme::Sockets::OUTLINE_SIZE, &mBlend
      );
      g.FillCircle(
        Theme::Sockets::COLOR_INNER, x, y,
        Theme::Sockets::RADIUS * 0.4, &mBlend
      );
    }

    void reverseDraw(IGraphics& g) {
      const float socketRadius = Theme::Sockets::DIAMETER / 2;
      for (int n = 0; n < mNodes->size(); n++) {
        NodeUi* curNode = mNodes->get(n);
        for (int i = 0; i < curNode->mNode->mOutputCount; i++) {
          NodeSocket* curSock = curNode->mNode->mSocketsOut[i];
          for (int j = 0; j < MAX_SOCKET_CONNECTIONS; j++) {
            if (curSock->mConnectedTo[j] != nullptr) {
              NodeSocket* tarSock = curSock->mConnectedTo[j];
              g.DrawLine(iplug::igraphics::COLOR_RED,
                curSock->mX + socketRadius, curSock->mY + socketRadius,
                tarSock->mX + socketRadius, tarSock->mY + socketRadius,
                &mBlend, 2
              );
            }
          }
        }
      }

    }

    void Draw(IGraphics& g) override {
      if (mOutNode == nullptr) { return; }
      const float socketRadius = Theme::Sockets::DIAMETER / 2;
      // Draw all the connections between nodes
      for (int n = 0; n < mNodes->size(); n++) {
        NodeUi* curNode = mNodes->get(n);
        for (int i = 0; i < curNode->mNode->mInputCount; i++) {
          NodeSocket* curSock = curNode->mNode->mSocketsIn[i];
          if (curSock->mConnectedTo[0] != nullptr) {
            NodeSocket* tarSock = curSock->mConnectedTo[0];
            if (tarSock == mPreviewSocket && curSock == mOutNode->mSocketsIn[0]) {
              // Draw the temporary bypass
              g.DrawDottedLine(
                curSock == mHighlightSocket ? Theme::Cables::COLOR_SPLICE_IN : Theme::Cables::COLOR,
                curSock->mX + socketRadius, curSock->mY + socketRadius,
                tarSock->mX + socketRadius, tarSock->mY + socketRadius,
                &mBlend, Theme::Cables::THICKNESS, Theme::Cables::PREVIEW_DASH_DIST
              );
              if (mPreviewSocketPrev != nullptr) {
                // draw the original connection slightly transparent
                g.DrawLine(
                  curSock == mHighlightSocket ? Theme::Cables::COLOR_SPLICE_IN : Theme::Cables::COLOR_PREVIEW,
                  curSock->mX + socketRadius, curSock->mY + socketRadius,
                  mPreviewSocketPrev->mX + socketRadius, mPreviewSocketPrev->mY + socketRadius,
                  &mBlend, Theme::Cables::THICKNESS
                );
              }
            }
            else {
              g.DrawLine(
                curSock == mHighlightSocket ? Theme::Cables::COLOR_SPLICE_IN : Theme::Cables::COLOR,
                curSock->mX + socketRadius, curSock->mY + socketRadius,
                tarSock->mX + socketRadius, tarSock->mY + socketRadius,
                &mBlend, Theme::Cables::THICKNESS
              );
            }
          }
        }
      }

      // Draw a new connection from the cursor to start socket
      if (mConnectionDragData != nullptr) {
        g.DrawLine(
          Theme::Cables::COLOR, mConnectionDragData->startX, mConnectionDragData->startY,
          mConnectionDragData->currentX, mConnectionDragData->currentY, &mBlend, Theme::Cables::THICKNESS
        );
      }

      // Draw all the sockets
      for (int n = 0; n < mNodes->size(); n++) {
        NodeUi* curNode = mNodes->get(n);
        for (int i = 0; i < curNode->mNode->mOutputCount; i++) {
          NodeSocket* curSock = curNode->mNode->mSocketsOut[i];
          if (curSock != nullptr) {
            DrawSocket(g, curSock);
          }
        }
        for (int i = 0; i < curNode->mNode->mInputCount; i++) {
          NodeSocket* curSock = curNode->mNode->mSocketsIn[i];
          if (curSock != nullptr) {
            DrawSocket(g, curSock);
          }
        }
      }
      //DrawSocket(g, mOutNode->mSocketsIn[0]);
      //DrawSocket(g, mInNode->shared.socketsOut[0]);

      // Visualizes the automation target node of each control attached to it
      Node* automation = mVisualizeAutomation;
      if (mPickAutomationTarget != nullptr) {
        automation = mPickAutomationTarget;
      }
      if (automation != nullptr) {
        for (int n = 0; n < mNodes->size(); n++) {
          NodeUi* curNode = mNodes->get(n);
          for (int p = 0; p < curNode->mNode->mParameterCount; p++) {
            ParameterCoupling* pc = &curNode->mNode->mParameters[p];
            if (pc->automationDependency == automation) {
              const IRECT pos = pc->control->GetRECT();
              g.DrawLine(
                Theme::Cables::AUTOMATION, pos.MW(), pos.MH(),
                pc->automationDependency->mPos.x, pc->automationDependency->mPos.y,
                &mBlend, Theme::Cables::THICKNESS
              );
            }
          }
        }
      }

      if (mPickAutomationTarget != nullptr) {
        g.FillRect(Theme::Cables::PICK_AUTOMATION, mRECT);
      }
#ifndef NDEBUG
      reverseDraw(g);
#endif

      if (mStats != nullptr && !mStats->valid) {
        const IRECT box = mRECT.GetPadded(-20).GetFromBottom(40);
        g.FillRect(Theme::Colors::ACCENT, box);
        g.DrawText(Theme::Gallery::CATEGORY_TITLE, "Graph cannot be computed (maybe it includes a cycle?)", box);
      }
    }

    void OnMouseDown(const float x, const float y, const IMouseMod& mod) override {
      if (mPickAutomationTarget != nullptr) {
        SetTargetRECT(IRECT(0, 0, 0, 0));
        IControl* target = GetUI()->GetControl(
          GetUI()->GetMouseControlIdx(x, y, false)
        );
        if (target != nullptr) {
          MessageBus::fireEvent<AutomationAttachRequest>(
            mBus, MessageBus::AttachAutomation,
            AutomationAttachRequest{
              mPickAutomationTarget, target
            }
          );
        }
        mPickAutomationTarget = nullptr;
        mDirty = true;
      }
      else {
        SetTargetRECT(IRECT(0, 0, 0, 0));
      }
    }

    void OnResize() override {
      mRECT.R = GetUI()->Width();
      mRECT.B = GetUI()->Height();
    }

  private:
    NodeSocket* getClosestToConnection(const Coord2D pos) const {
      const float socketRadius = Theme::Sockets::DIAMETER / 2;
      for (int n = 0; n < mNodes->size(); n++) {
        NodeUi* curNode = mNodes->get(n);
        for (int i = 0; i < curNode->mNode->mInputCount; i++) {
          NodeSocket* curSock = curNode->mNode->mSocketsIn[i];
          NodeSocket* tarSock = curSock->mConnectedTo[0];
          if (tarSock != nullptr) {
            float x1 = tarSock->mX + socketRadius;
            float x2 = curSock->mX + socketRadius;
            float y1 = tarSock->mY + socketRadius;
            float y2 = curSock->mY + socketRadius;
            IRECT box;
            box.L = std::min(x1, x2) - SPLICEIN_DISTANCE;
            box.R = std::max(x1, x2) + SPLICEIN_DISTANCE;
            box.T = std::min(y1, y2) - SPLICEIN_DISTANCE;
            box.B = std::max(y1, y2) + SPLICEIN_DISTANCE;
            if (box.Contains(IRECT{ pos.x, pos.y, pos.x, pos.y })) {
              const float a = y1 - y2;
              const float b = x2 - x1;
              const float c = x1 * y2 - x2 * y1;
              const float d = abs(a * pos.x + b * pos.y + c) / sqrt(a * a + b * b);
              if (d < SPLICEIN_DISTANCE) {
                return curSock;
              }
            }
          }
        }
      }
      return nullptr;
    }
  };
}
#endif