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
    IGraphics* mGraphics = nullptr;
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

      mBackground = new GraphBackground(mBus, mGraphics, [&](float x, float y, float scale) {
        this->onViewPortChange(x, y, scale);
      });
      mGraphics->AttachControl(mBackground);

      mSideBar = new SideBar(mBus, mGraphics);
      mGraphics->AttachControl(mSideBar);

      mCableLayer = new CableLayer(mBus, mGraphics, &mNodeUis);
      mCableLayer->SetRenderPriority(10);
      mGraphics->AttachControl(mCableLayer);
    }

    ~GraphUi() {
      mSelectedNodes.clear();
      mGraph->setScale(mGraphics->GetDrawScale());

      for (int n = 0; n < mNodeUis.size(); n++) {
        cleanUpNodeUi(mNodeUis[n]->shared->node);
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
        cleanUpNodeUi(mNodeUis[0]->shared->node);
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
      mCableLayer->setInOutNodes(mInputNodeUi->shared->node, moutputNodeUi->shared->node);

      const float scale = mGraph->getScale();
      mBackground->mScale = scale;
      //mGraphics->Resize(mGraphics->Width(), mGraphics->Height(), scale); TODO move this in the plugin
      return scale;
    }

    void addNode(
      Node* node, Node* pInput = nullptr, const float x = 0, const float y = 0,
      const int outputIndex = 0, const int inputIndex = 0, Node* clone = nullptr
    ) {
      mGraph->addNode(node, pInput, x, y, outputIndex, inputIndex, clone);
      setUpNodeUi(node);
    }

    void removeNode(Node* node, const bool reconnect = false) {
      cleanUpNodeUi(node);
      mGraph->removeNode(node, reconnect);
    }

    void cleanUpNodeUi(Node* node) {
      for (size_t i = 0; i < mNodeUis.size(); i++) {
        if (mNodeUis[i] == node->mUi) {
          node->cleanupUi(mGraphics);
          mNodeUis.remove(i);
          return;
        }
      }
    }

    NodeUi* setUpNodeUi(Node* node) {
      node->setupUi(mGraphics);
      mNodeUis.add(node->mUi);
      return node->mUi;
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

    /**
     * Called via a callback from the background to move around all the nodes
     * creating the illusion of a viewport
     */
    void onViewPortChange(const float dX = 0, const float dY = 0, float scale = 1) const {
      for (int i = 0; i < mNodeUis.size(); i++) {
        mNodeUis[i]->translate(dX, dY);
      }
      // mOutputNode->mUi->translate(dX, dY);
      //mInputNode->mUi->translate(dX, dY);
      // WDBGMSG("x %f y %f s %f\n", x, y, scale);
    }

    /**
     * Centers the viewport around a specific node
     */
    void centerNode(NodeUi* node) const {
      IRECT center = mGraphics->GetBounds().GetScaledAboutCentre(0);
      center.L -= node->shared->X;
      center.T -= node->shared->Y;
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
        avg.x += n->shared->X;
        avg.y += n->shared->Y;
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
        this->addNode(NodeList::createNode(info.name), nullptr, 300, 300);
      });

      mNodeDelSub.subscribe(mBus, MessageBus::NodeDeleted, [&](Node* param) {
        MessageBus::fireEvent(mBus, MessageBus::PushUndoState, false);
        this->removeNode(param, true);
      });

      mNodeBypassEvent.subscribe(mBus, MessageBus::BypassNodeConnection, [&](Node* param) {
        MessageBus::fireEvent(mBus, MessageBus::PushUndoState, false);
        mGraph->byPassConnection(param);
      });

      mNodeCloneEvent.subscribe(mBus, MessageBus::CloneNode, [&](Node* node) {
        Node* clone = NodeList::createNode(node->shared.info->name);
        if (clone != nullptr) {
          this->addNode(clone, nullptr, node->shared.X, node->shared.Y, 0, 0, node);
          clone->mUi->mDragging = true;
          mGraphics->SetCapturedControl(clone->mUi);
          MessageBus::fireEvent<NodeSelectionChanged>(
            mBus, MessageBus::NodeSelectionChange, { clone->mUi, true }
          );
        }
      });

      mNodeDragSpawn.subscribe(mBus, MessageBus::NodeDragSpawn, [&](NodeDragSpawnRequest req) {
        Node* node = NodeList::createNode(req.name);
        if (node != nullptr) {
          this->addNode(node, nullptr, req.pos.x, req.pos.y, 0, 0);
          node->mUi->mDragging = true;
          mGraphics->SetCapturedControl(node->mUi);
          MessageBus::fireEvent<NodeSelectionChanged>(
            mBus, MessageBus::NodeSelectionChange, { node->mUi, true }
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
          setUpNodeUi(combine);
          combine->mUi->mDragging = true;
          mGraphics->SetCapturedControl(combine->mUi);
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
          Node* node = mNodeUis[i]->shared->node;
          for (int p = 0; p < node->shared.parameterCount; p++) {
            if (node->shared.parameters[p].control == r.targetControl) {
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
          Node* in = mInputNodeUi->shared->node;
          resetBranchPos(in);
          arrangeBranch(in, Coord2D{ in->shared.Y, in->shared.X });
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
    static void resetBranchPos(Node* node) {
      if (node == nullptr || node->shared.info->name == "FeedbackNode") { return; }
      node->mUi->setTranslation(0, 0);
      NodeSocket* socket = nullptr;
      for (int i = 0; i < node->shared.outputCount; i++) {
        socket = node->shared.socketsOut[i];
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
    static Coord2D arrangeBranch(Node* node, Coord2D pos) {
      if (node == nullptr || node->shared.info->name == "FeedbackNode") {
        return pos;
      }
      const float halfWidth = node->shared.width * 0.5;
      // const float halfHeight = node->shared.height * 0.5;
      const float padding = 50;
      pos.x += halfWidth + padding;
      node->mUi->setTranslation(pos.x, pos.y);
      pos.x += halfWidth + padding;
      float nextX = 0;
      NodeSocket* socket = nullptr;
      for (int i = 0; i < node->shared.outputCount; i++) {
        socket = node->shared.socketsOut[i];
        for (int j = 0; j < MAX_SOCKET_CONNECTIONS; j++) {
          if (socket->mConnectedTo[j] != nullptr) {
            if (socket->mConnectedTo[j]->mIndex == 0) {
              Coord2D branch = arrangeBranch(socket->mConnectedTo[j]->mParentNode, pos);
              pos.y += node->shared.height + padding;
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