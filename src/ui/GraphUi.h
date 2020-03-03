#pragma once

#include "./GraphBackground.h"
#include "./CableLayer.h"
#include "./SideBar.h"
#include "../misc/HistoryStack.h"
#include "../graph/Graph.h"
#include "../node/NodeUi.h"

namespace guitard {
  /**
   * This is the central GUI class of the Plugin
   * There will only be on instance and only if the GUI is open
   * It wraps around and controls a Graph (which in theory can be swapped)
   * It's not deriving from Graph, since the Graph needs to be able to
   * exist without this object
   */
  class GraphUi {
    MessageBus::Bus* mBus = nullptr;
    IGraphics* mGraphics = nullptr; // Since this is not a UI element, well need to keep the graphics around
    Graph* mGraph = nullptr; // This is the Graph object which will be manipulated from this UI object
    PointerList<NodeUi> mNodeUis; // Keep around all the node UIs

    /**
     * Whole lot of subscriptions needed for the graph
     */
    MessageBus::Subscription<Node*> mNodeDelSub;
    MessageBus::Subscription<Node*> mNodeBypassEvent;
    MessageBus::Subscription<Node*> mNodeCloneEvent;
    MessageBus::Subscription<NodeDragSpawnRequest> mNodeDragSpawn;
    MessageBus::Subscription<Node*> mNodeSpliceCombineEvent;
    MessageBus::Subscription<NodeList::NodeInfo> mNodeAddEvent;
    MessageBus::Subscription<bool> mAwaitAudioMutexEvent;
    MessageBus::Subscription<bool> mPushUndoState;
    MessageBus::Subscription<bool> mPopUndoState;
    MessageBus::Subscription<GraphStats**> mReturnStats;
    MessageBus::Subscription<AutomationAttachRequest> mAutomationRequest;
    MessageBus::Subscription<const char*> mLoadPresetEvent;
    MessageBus::Subscription<WDL_String*> mSavePresetEvent;
    MessageBus::Subscription<BlockSizeEvent*> mMaxBlockSizeEvent;
    MessageBus::Subscription<NodeSelectionChanged> mSelectionChagedEvent;
    MessageBus::Subscription<Drag> mNodeDragged;

    /**
     * Control elements
     */
    GraphBackground* mBackground = nullptr; // Always at the bottom
    CableLayer* mCableLayer = nullptr; // Always below the Gallery
    SideBar* mSideBar = nullptr; // Always top most

    HistoryStack mHistoryStack;

    PointerList<NodeUi> mSelectedNodes;

    NodeUi* mInputNodeUi = nullptr;
    NodeUi* moutputNodeUi = nullptr;
  public:
    GraphUi(MessageBus::Bus* bus, IGraphics* graphics) {
      mBus = bus;
      initSubscriptions();
      mGraphics = graphics;
      mGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Size, true);
      mGraphics->EnableMouseOver(true);
      mGraphics->AttachTextEntryControl();
      mGraphics->AttachPopupMenuControl(iplug::igraphics::DEFAULT_LABEL_TEXT);

      mGraphics->SetKeyHandlerFunc([&](const IKeyPress& key, const bool isUp) {
        return handleKeyEvent(key, isUp);
      });

      mBackground = new GraphBackground(mBus, [&](float x, float y, float scale) {
        this->onViewPortChange(x, y, scale);
      });
      mGraphics->AttachControl(mBackground);

      mSideBar = new SideBar(mBus);
      mGraphics->AttachControl(mSideBar);

      mCableLayer = new CableLayer(mBus, &mNodeUis);
      mCableLayer->SetRenderPriority(10);
      mGraphics->AttachControl(mCableLayer);
    }

    ~GraphUi() {
      mSelectedNodes.clear();
      mGraph->setScale(mGraphics->GetDrawScale());

      for (int n = 0; n < mNodeUis.size(); n++) {
        cleanUpNodeUi(mNodeUis[n]->mNode);
      }

      mGraphics->RemoveControl(mSideBar);
      mSideBar = nullptr;

      mGraphics->RemoveControl(mBackground);
      mBackground = nullptr;

      mGraphics->RemoveControl(mCableLayer);
      mCableLayer = nullptr;

      mGraphics->RemoveAllControls();
      mGraphics = nullptr;
    }

    void cleanUpAllNodeUis() {
      mCableLayer->setInOutNodes(nullptr, nullptr);
      while (mNodeUis.size()) {
        cleanUpNodeUi(mNodeUis[0]->mNode);
      }
    }

    /**
     * Sets the graph provided and returns the scale the graph should be drawn with
     */
    float setGraph(Graph* graph) {
      cleanUpAllNodeUis();
      mGraph = graph;
      PointerList<Node> nodes = mGraph->getNodes();
      for (int n = 0; n < nodes.size(); n++) {
        setUpNodeUi(nodes[n]);
      }
      mInputNodeUi = setUpNodeUi(mGraph->getInputNode());
      moutputNodeUi = setUpNodeUi(mGraph->getOutputNode());
      mCableLayer->setInOutNodes(mInputNodeUi->mNode, moutputNodeUi->mNode);

      const float scale = mGraph->getScale();
      mBackground->mScale = scale;
      //mGraphics->Resize(mGraphics->Width(), mGraphics->Height(), scale); TODO move this in the plugin
      return scale;
    }

    void cleanUpNodeUi(Node* node) {
      NodeUi* ui = getUiFromNode(node);
      if (ui != nullptr) {
        mNodeUis.remove(ui);
        mGraphics->RemoveControl(ui);
      }
    }

    NodeUi* setUpNodeUi(Node* node) {
      NodeUi* ui = NodeList::createNodeUi(node->mInfo->name, node, mBus);
      if (ui != nullptr) {
        mGraphics->AttachControl(ui);
        mNodeUis.add(ui);
        ui->setUp();
      }
      return ui;
    }

    void deserialize(const char* data) {
      cleanUpAllNodeUis();
      mGraph->deserialize(data);
      setGraph(mGraph);
    }

    void deserialize(nlohmann::json& json) {
      cleanUpAllNodeUis();
      mGraph->deserialize(json);
      setGraph(mGraph);
    }

  private:
    Node* getNodeFromUi(NodeUi* ui) const {
      return ui->mNode;
    }

    NodeUi* getUiFromNode(Node* node) const {
      for (size_t i = 0; i < mNodeUis.size(); i++) {
        if (mNodeUis[i]->mNode == node) {
          return mNodeUis[i];
        }
      }
      return nullptr;
    }

    /**
     * Called via a callback from the background to move around all the nodes
     * creating the illusion of a viewport
     */
    void onViewPortChange(const float dX = 0, const float dY = 0, float scale = 1) const {
      for (int i = 0; i < mNodeUis.size(); i++) {
        NodeUi* ui = mNodeUis[i];
        ui->translate(dX, dY);
      }
    }

    /**
     * Centers the viewport around a specific node
     */
    void centerNode(NodeUi* node) const {
      IRECT center = mGraphics->GetBounds().GetScaledAboutCentre(0);
      center.L -= node->mNode->mPos.x;
      center.T -= node->mNode->mPos.y;
      onViewPortChange(center.L, center.T);
    }

    /**
     * Averages all node positions and moves the viewport to that point
     * Bound to the C key
     */
    void centerGraph() const {
      Coord2D avg{ 0, 0 };
      const int count = mNodeUis.size();
      for (int i = 0; i < count; i++) {
        const NodeUi* n = mNodeUis[i];
        avg.x += n->mNode->mPos.x;
        avg.y += n->mNode->mPos.y;
      }
      float countf = count /** + 2 */;
      //avg.x += mInputNode->shared.X + mOutputNode->shared.X;
      //avg.y += mInputNode->shared.Y + mOutputNode->shared.Y;
      // We want that point to be in the center of the screen
      const IRECT center = mGraphics->GetBounds().GetScaledAboutCentre(0);
      avg.x = center.L - avg.x / countf;
      avg.y = center.T - avg.y / countf;
      onViewPortChange(avg.x, avg.y);
    }

    void initSubscriptions() {
      /**
       * All the events the Graph is subscribed to
       */
      mNodeAddEvent.subscribe(mBus, MessageBus::NodeAdd, [&](const NodeList::NodeInfo& info) {
        MessageBus::fireEvent(mBus, MessageBus::PushUndoState, false);
        Node* node = NodeList::createNode(info.name);
        mGraph->addNode(node, nullptr, { 300, 300 });
        setUpNodeUi(node);
      });

      mNodeDelSub.subscribe(mBus, MessageBus::NodeDeleted, [&](Node* param) {
        MessageBus::fireEvent(mBus, MessageBus::PushUndoState, false);
        cleanUpNodeUi(param);
        mGraph->removeNode(param, true);
      });

      mNodeBypassEvent.subscribe(mBus, MessageBus::BypassNodeConnection, [&](Node* param) {
        MessageBus::fireEvent(mBus, MessageBus::PushUndoState, false);
        mGraph->byPassConnection(param);
      });

      mNodeCloneEvent.subscribe(mBus, MessageBus::CloneNode, [&](Node* node) {
        Node* clone = NodeList::createNode(node->mInfo->name);
        if (clone != nullptr) {
          mGraph->addNode(clone, nullptr, node->mPos, 0, 0, node);
          NodeUi* ui = setUpNodeUi(clone);
          ui->mDragging = true;
          mGraphics->SetCapturedControl(ui);
          MessageBus::fireEvent<NodeSelectionChanged>(
            mBus, MessageBus::NodeSelectionChange, { ui, true }
          );
        }
      });

      mNodeDragSpawn.subscribe(mBus, MessageBus::NodeDragSpawn, [&](NodeDragSpawnRequest req) {
        Node* node = NodeList::createNode(req.name);
        if (node != nullptr) {
          mGraph->addNode(node, nullptr, req.pos, 0, 0);
          NodeUi* ui = setUpNodeUi(node);
          ui->mDragging = true;
          mGraphics->SetCapturedControl(ui);
          MessageBus::fireEvent<NodeSelectionChanged>(
            mBus, MessageBus::NodeSelectionChange, { ui, true }
          );
        }
      });

      mPushUndoState.subscribe(mBus, MessageBus::PushUndoState, [&](bool) {
        WDBGMSG("PushState");
        mGraph->serialize(*(mHistoryStack.pushState()));
      });

      mPopUndoState.subscribe(mBus, MessageBus::PopUndoState, [&](const bool redo) {
        nlohmann::json* state = mHistoryStack.popState(redo);
        if (state != nullptr) {
          WDBGMSG("PopState");
          this->deserialize(*state);
        }
      });

      //mReturnStats.subscribe(mBus, MessageBus::GetGraphStats, [&](GraphStats** stats) {
      //  *stats = &mStats;
      //});

      mNodeSpliceCombineEvent.subscribe(mBus, MessageBus::NodeSpliceInCombine, [&](Node* node) {
        // this->spliceInCombine(node);
        Node* combine = mGraph->spliceInCombine(node);
        if (combine != nullptr) {
          NodeUi* ui = setUpNodeUi(combine);
          ui->mDragging = true;
          mGraphics->SetCapturedControl(ui);
        }
      });

      mLoadPresetEvent.subscribe(mBus, MessageBus::LoadPresetFromString, [&](const char* data) {
        deserialize(data);
      });

      mSavePresetEvent.subscribe(mBus, MessageBus::SavePresetToSring, [&](WDL_String* data) {
        mGraph->serialize(*data);
      });

      mAutomationRequest.subscribe(mBus, MessageBus::AttachAutomation, [&](AutomationAttachRequest r) {
        MessageBus::fireEvent(mBus, MessageBus::PushUndoState, false);
        for (int i = 0; i < mNodeUis.size(); i++) {
          Node* node = mNodeUis[i]->mNode;
          for (int p = 0; p < node->mParameterCount; p++) {
            if (node->mParameters[p].control == r.targetControl) {
              if (node != r.automationNode) {
                // Don't allow automation on self
                node->attachAutomation(r.automationNode, p);
              }
            }
          }
        }
      });

      mMaxBlockSizeEvent.subscribe(mBus, MessageBus::MaxBlockSizeEvent, [&](BlockSizeEvent* e) {
        if (e->set) {
          mGraph->setBlockSize(e->blockSize);
        }
        //else { TODOG have a look at this again
        //  e->blockSize = this->mMaxBlockSize;
        //}
      });

      mAwaitAudioMutexEvent.subscribe(mBus, MessageBus::AwaitAudioMutex, [&](const bool doPause) {
        if (doPause) {
          mGraph->lockAudioThread();
        }
        else {
          mGraph->unlockAudioThread();
        }
      });

      mSelectionChagedEvent.subscribe(mBus, MessageBus::NodeSelectionChange, [&](NodeSelectionChanged event) {
        if (event.remove) {
          event.node->setSelected(false);
          mSelectedNodes.remove(event.node);
          return;
        }
        if (event.replace) { // replace whole selection
          for (size_t i = 0; i < mSelectedNodes.size(); i++) {
            mSelectedNodes[i]->setSelected(false);
          }
          mSelectedNodes.clear();
          if (event.node != nullptr) { // clear it completly
            event.node->setSelected(true);
            mSelectedNodes.add(event.node); // Replace Selection
          }
        }
        else { // toggle selection
          if (mSelectedNodes.find(event.node) == -1) {
            event.node->setSelected(true);
            mSelectedNodes.add(event.node); // Wasn't selected, add now
          }
          else {
            event.node->setSelected(false);
            mSelectedNodes.remove(event.node); // Was selected, remove now
          }
        }
      });

      mNodeDragged.subscribe(mBus, MessageBus::NodeDragged, [&](const Drag drag) {
        for (int i = 0; i < mSelectedNodes.size(); i++) {
          mSelectedNodes[i]->translate(drag.delta.x, drag.delta.y);
        }
      });
    }

    bool handleKeyEvent(const IKeyPress& key, const bool isUp) {
      // Gets the keystrokes in the standalone app
      if (!isUp) { // Only handle key down
        if (key.S) { // Check modifiers like shift first
          if (key.VK == iplug::kVK_Z) {
            MessageBus::fireEvent<bool>(this->mBus, MessageBus::PopUndoState, false);
            return true;
          }
          if (key.VK == iplug::kVK_C) {
            WDL_String data;
            mGraph->serialize(data);
            mGraphics->SetTextInClipboard(data);
            return true;
          }
          if (key.VK == iplug::kVK_V) {
            WDL_String data;
            mGraphics->GetTextFromClipboard(data);
            deserialize(data.Get());
            return true;
          }
        }
        if (key.VK == iplug::kVK_F) {
          Node* in = mInputNodeUi->mNode;
          resetBranchPos(in);
          arrangeBranch(in, in->mPos);
          centerGraph();
          return true;
        }
        if (key.VK == iplug::kVK_C) {
          this->centerGraph();
          return true;
        }
        if (key.VK == iplug::kVK_Q) {
          centerNode(mInputNodeUi);
          return true;
        }
        if (key.VK == iplug::kVK_E) {
          centerNode(moutputNodeUi);
          return true;
        }
        if (key.VK == iplug::kVK_S) {
          mGraph->sortGraph();
          return true;
        }
      }
      return false;
    }

    // void spliceInCombine

    /**
     * Recursively resets all the positions of nodes to (0, 0)
     */
    void resetBranchPos(Node* node) {
      if (node == nullptr || node->mInfo->name == "FeedbackNode") { return; }
      getUiFromNode(node)->setTranslation(0, 0);
      NodeSocket* socket = nullptr;
      for (int i = 0; i < node->mOutputCount; i++) {
        socket = node->mSocketsOut[i];
        for (int j = 0; j < MAX_SOCKET_CONNECTIONS; j++) {
          if (socket->mConnectedTo[j] != nullptr) {
            if (socket->mConnectedTo[j]->mIndex == 0) {
              resetBranchPos(socket->mConnectedTo[j]->mParentNode);
            }
          }
        }
      }
    }

    /**
     * Recursively sorts nodes. I don't even know what's going on here, but it works. Sort of
     */
    Coord2D arrangeBranch(Node* node, Coord2D pos) {
      if (node == nullptr || node->mInfo->name == "FeedbackNode") {
        return pos;
      }
      const float halfWidth = node->mDimensions.x * 0.5;
      // const float halfHeight = node->shared.height * 0.5;
      const float padding = 50;
      pos.x += halfWidth + padding;
      getUiFromNode(node)->setTranslation(pos.x, pos.y);
      pos.x += halfWidth + padding;
      float nextX = 0;
      NodeSocket* socket = nullptr;
      for (int i = 0; i < node->mOutputCount; i++) {
        socket = node->mSocketsOut[i];
        for (int j = 0; j < MAX_SOCKET_CONNECTIONS; j++) {
          if (socket->mConnectedTo[j] != nullptr) {
            if (socket->mConnectedTo[j]->mIndex == 0) {
              Coord2D branch = arrangeBranch(socket->mConnectedTo[j]->mParentNode, pos);
              pos.y += node->mDimensions.y + padding;
              if (pos.y < branch.y) {
                pos.y = branch.y;
              }
              if (branch.x > nextX) {
                nextX = branch.x;
              }
            }
          }
        }
      }
      return Coord2D{ nextX, pos.y };
    }
  };

}