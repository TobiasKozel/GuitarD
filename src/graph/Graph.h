#pragma once
#include <chrono>
#include "json.hpp"
#include "src/misc/constants.h"
#include "src/types/gmutex.h"
#include "src/types/types.h"
#include "src/types/pointerList.h"
#include "src/misc/MessageBus.h"
#include "src/nodes/io/InputNode.h"
#include "src/nodes/io/OutputNode.h"
#include "Serializer.h"
#include "src/parameter/ParameterManager.h"

#ifndef GUITARD_HEADLESS
#include "src/ui/GraphBackground.h"
#include "SortGraph.h" 
#include "src/ui/CableLayer.h"
#include "src/ui/SideBar.h"
#include "src/misc/HistoryStack.h"
#include "FormatGraph.h"
#endif

namespace guitard {
  /**
   * This is the "god object" which will handle all the nodes
   * and interactions with the graph
   * It's not a IControl itself but owns a few which make up the GUI
   */
  class Graph {
    MessageBus::Bus* mBus = nullptr;


#ifndef GUITARD_HEADLESS
    /**
     * Whole lot of subscriptions needed for the graph
     */
    MessageBus::Subscription<Node*> mNodeDelSub;
    MessageBus::Subscription<Node*> mNodeBypassEvent;
    MessageBus::Subscription<Node*> mNodeCloneEvent;
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
    IGraphics* mGraphics = nullptr;

    /**
     * Control elements
     */
    GraphBackground* mBackground = nullptr; // Always at the bottom
    CableLayer* mCableLayer = nullptr; // Always below the Gallery
    SideBar* mSideBar = nullptr; // Always top most

    HistoryStack mHistoryStack;

    /**
     * Editor window properties
     * Kept around for the serialization
     */
    int mWindowWidth = 0;
    int mWindowHeight = 0;
    float mWindowScale = 0;
#endif

    ParameterManager* mParamManager = nullptr;

    /**
     * Holds all the nodes in the processing graph
     */
    PointerList<Node> mNodes;

    /**
     * Mutex to keep changes to the graph like adding/removing or rerouting from crashing
     */
    Mutex mAudioMutex;

    /**
     * Acts as a semaphore since the mAudioMutex only needs to be locked once to stop the audio thread
     */
    int mPauseAudio = 0;

    /**
     * Dummy nodes to get the audio blocks in and out of the graph
     */
    InputNode* mInputNode;
    OutputNode* mOutputNode;

    /**
     * This is the channel count to be used internally
     * All nodes will allocate buffers to according to this
     * Using anything besides stereo will cause problems with the faust DSP code
     */
    int mChannelCount = 0;

    /**
     * This is the actual input channel count provided by the DAW
     * The input channel count can be 1 if the plugin is on a mono track
     * But internal processing will still happen in stereo
     */
    int mInPutChannelCount = 0;

    int mSampleRate = 0;

    /**
     * The max blockSize which can be used to minimize round trip delay when having cycles in the graph
     */
    int mMaxBlockSize = MAX_BUFFER;



    GraphStats mStats;

    /**
     * Used to slice the dsp block in smaller slices
     */
    sample** mSliceBuffer[2] = { nullptr };

  public:

    explicit Graph(MessageBus::Bus* pBus, ParameterManager* pParamManager) {
      mBus = pBus;
      mParamManager = pParamManager;

      mInputNode = new InputNode(mBus);
      mOutputNode = new OutputNode(mBus);
      mOutputNode->connectInput(mInputNode->shared.socketsOut[0]);

#ifndef GUITARD_HEADLESS
      /**
       * All the events the Graph is subscribed to, they're only needed with a gui
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
        this->byPassConnection(param);
      });



      mNodeCloneEvent.subscribe(mBus, MessageBus::CloneNode, [&](Node* node) {
        Node* clone = NodeList::createNode(node->shared.type);
        if (clone != nullptr) {
          this->addNode(clone, nullptr, node->shared.X, node->shared.Y, 0, 0, node);
          clone->mUi->mDragging = true;
          mGraphics->SetCapturedControl(clone->mUi);
        }
      });

      mPushUndoState.subscribe(mBus, MessageBus::PushUndoState, [&](bool) {
        WDBGMSG("PushState");
        this->serialize(*(mHistoryStack.pushState()));
      });

      mPopUndoState.subscribe(mBus, MessageBus::PopUndoState, [&](const bool redo) {
        nlohmann::json* state = mHistoryStack.popState(redo);
        if (state != nullptr) {
          WDBGMSG("PopState");
          this->deserialize(*state);
        }
      });

      mReturnStats.subscribe(mBus, MessageBus::GetGraphStats, [&](GraphStats** stats) {
        *stats = &mStats;
      });

      mNodeSpliceCombineEvent.subscribe(mBus, MessageBus::NodeSpliceInCombine, [&](Node* node) {
        this->spliceInCombine(node);
      });

      mLoadPresetEvent.subscribe(mBus, MessageBus::LoadPresetFromString, [&](const char* data) {
        this->deserialize(data);
      });

      mSavePresetEvent.subscribe(mBus, MessageBus::SavePresetToSring, [&](WDL_String* data) {
        this->serialize(*data);
      });

      mAutomationRequest.subscribe(mBus, MessageBus::AttachAutomation, [&](AutomationAttachRequest r) {
        MessageBus::fireEvent(mBus, MessageBus::PushUndoState, false);
        PointerList<Node>& n = this->mNodes;
        for (int i = 0; i < n.size(); i++) {
          Node* node = n[i];
          if (node == nullptr) { continue; }
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
          this->setBlockSize(e->blockSize);
        }
        //else {
        //  e->blockSize = this->mMaxBlockSize;
        //}
      });

      mAwaitAudioMutexEvent.subscribe(mBus, MessageBus::AwaitAudioMutex, [&](const bool doPause) {
        if (doPause) {
          this->lockAudioThread();
        }
        else {
          this->unlockAudioThread();
        }
    });
#endif
    }

    ~Graph() {
      removeAllNodes();
      // TODOG get rid of all the things
    }

    void lockAudioThread() {
      if (mPauseAudio == 0) {
        mAudioMutex.lock();
      }
      mPauseAudio++;
    }

    void unlockAudioThread() {
      if (mPauseAudio == 1) {
        mAudioMutex.unlock();
      }
      mPauseAudio--;
    }

    void OnReset(const int pSampleRate, const int pOutputChannels = 2, const int pInputChannels = 2) {
      if (pSampleRate != mSampleRate || pOutputChannels != mChannelCount || pInputChannels != mInPutChannelCount) {
        lockAudioThread();
        mSampleRate = pSampleRate;
        {
          if (mSliceBuffer[0] != nullptr) {
            for (int c = 0; c < pOutputChannels; c++) {
              delete mSliceBuffer[0];
              mSliceBuffer[0] = nullptr;
              delete mSliceBuffer[1];
              mSliceBuffer[1] = nullptr;
            }
          }
          mSliceBuffer[0] = new sample * [pOutputChannels];
          mSliceBuffer[1] = new sample * [pOutputChannels];
          /**
           * There's no need to create a buffer inside since it will use the pointer from the
           * scratch buffer offset by the sub-block position
           */
        }
        mChannelCount = pOutputChannels;
        mInPutChannelCount = pInputChannels;
        mInputNode->setInputChannels(pInputChannels);
        mInputNode->OnReset(pSampleRate, pOutputChannels);
        mOutputNode->OnReset(pSampleRate, pOutputChannels);
        for (int i = 0; i < mNodes.size(); i++) {
          mNodes[i]->OnReset(pSampleRate, pOutputChannels);
        }
        unlockAudioThread();
      }
      else {
        /**
         * If nothing has changed we'll assume a transport
         */
        OnTransport();
      }
    }

    void OnTransport() {
      lockAudioThread();
      mInputNode->OnTransport();
      mOutputNode->OnTransport();
      for (int i = 0; i < mNodes.size(); i++) {
        mNodes[i]->OnTransport();
      }
      unlockAudioThread();
    }

    void setBlockSize(const int size) {
      if (size == mMaxBlockSize || size > MAX_BUFFER) { return; }
      lockAudioThread();
      mMaxBlockSize = size;
      for (int i = 0; i < mNodes.size(); i++) {
        Node* n = mNodes[i];
        n->shared.maxBlockSize = size;
        n->OnReset(mSampleRate, mChannelCount, true);
      }
      unlockAudioThread();
    }

    /**
     * Main entry point for the DSP
     */
    void ProcessBlock(sample** in, sample** out, const int nFrames) {
      /**
       * Process the block in smaller bits since it's too large
       * Also abused to lower the delay a feedback node creates
       */
      if (nFrames > mMaxBlockSize) {
        const int overhang = nFrames % mMaxBlockSize;
        int s = 0;
        while (true) {
          for (int c = 0; c < mChannelCount; c++) {
            mSliceBuffer[0][c] = &in[c][s];
            mSliceBuffer[1][c] = &out[c][s];
          }
          s += mMaxBlockSize;
          if (s <= nFrames) {
            ProcessBlock(mSliceBuffer[0], mSliceBuffer[1], mMaxBlockSize);
          }
          else {
            if (overhang > 0) {
              ProcessBlock(mSliceBuffer[0], mSliceBuffer[1], overhang);
            }
            return;
          }
        }
      }


      const int nodeCount = mNodes.size();
      const int maxAttempts = 10;

      /**
       * Do a version without mutex and stats if there's no gui
       * since there is no need for locking if the graph can't be altered
       */
#ifndef GUITARD_HEADLESS
      if (mGraphics == nullptr)
#endif
      {
        mInputNode->CopyIn(in, nFrames);
        for (int n = 0; n < nodeCount; n++) {
          mNodes[n]->BlockStart();
        }
        mOutputNode->BlockStart();

        // The List is pre sorted so the attempts are only needed to catch circular dependencies and other edge cases

        int attempts = 0;
        while (!mOutputNode->mIsProcessed && attempts < maxAttempts) {
          for (int n = 0; n < nodeCount; n++) {
            mNodes[n]->ProcessBlock(nFrames);
          }
          mOutputNode->ProcessBlock(nFrames);
          attempts++;
        }

        if (attempts < maxAttempts) {
          for (int n = 0; n < nodeCount; n++) {
            mNodes[n]->ProcessBlock(nFrames);
          }
        }
        mOutputNode->CopyOut(out, nFrames);
        return;
      }
      {
        /**
         * The version with mutex locking
         */
        if (mPauseAudio > 0) {
          /**
           * Skip the block if the mutex is locked, waiting will most likely result in an under-run anyways
           */
          for (int c = 0; c < mChannelCount; c++) {
            for (int i = 0; i < nFrames; i++) {
              out[c][i] = 0;
            }
          }
          return;
        }

        const auto start = std::chrono::high_resolution_clock::now();
        LockGuard lock(mAudioMutex);
        mInputNode->CopyIn(in, nFrames);
        for (int n = 0; n < nodeCount; n++) {
          mNodes[n]->BlockStart();
        }
        mOutputNode->BlockStart();
        // The List is pre sorted so the attempts are only needed to catch circular dependencies and other edge cases
        int attempts = 0;
        while (!mOutputNode->mIsProcessed && attempts < maxAttempts) {
          for (int n = 0; n < nodeCount; n++) {
            mNodes[n]->ProcessBlock(nFrames);
          }
          mOutputNode->ProcessBlock(nFrames);
          attempts++;
        }

        // This extra iteration makes sure the feedback loops get data from their previous nodes
        if (attempts < maxAttempts) {
          for (int n = 0; n < nodeCount; n++) {
            mNodes[n]->ProcessBlock(nFrames);
          }
          if (!mStats.valid) {
            mStats.valid = true;
            MessageBus::fireEvent(mBus, MessageBus::GraphStatsChanged, &mStats);
          }
        }
        else {
          // failed processing
          if (mStats.valid) {
            mStats.valid = false;
            MessageBus::fireEvent(mBus, MessageBus::GraphStatsChanged, &mStats);
          }
        }

        mOutputNode->CopyOut(out, nFrames);
        const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::high_resolution_clock::now() - start
          );
        mStats.executionTime = duration.count();
      }
    }

#ifndef GUITARD_HEADLESS
    /**
     * The graph needs to know about the graphics context to add and remove the controls for the nodes
     * It also handles keystrokes globally
     */
    void setupUi(iplug::igraphics::IGraphics* pGraphics = nullptr) {
      if (pGraphics != nullptr && pGraphics != mGraphics) {
        WDBGMSG("Graphics context changed");
        mGraphics = pGraphics;
      }
      pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Size, true);
      pGraphics->EnableMouseOver(true);
      pGraphics->AttachTextEntryControl();
      pGraphics->AttachPopupMenuControl(iplug::igraphics::DEFAULT_LABEL_TEXT);

      mGraphics->SetKeyHandlerFunc([&](const IKeyPress& key, const bool isUp) {
        // Gets the keystrokes in the standalone app
        if (!isUp) { // Only handle key down
          if (key.S) { // Check modifiers like shift first
            if (key.VK == iplug::kVK_Z) {
              MessageBus::fireEvent<bool>(this->mBus, MessageBus::PopUndoState, false);
              return true;
            }
            if (key.VK == iplug::kVK_C) {
              WDL_String data;
              this->serialize(data);
              this->mGraphics->SetTextInClipboard(data);
              return true;
            }
            if (key.VK == iplug::kVK_V) {
              WDL_String data;
              this->mGraphics->GetTextFromClipboard(data);
              this->deserialize(data.Get());
              return true;
            }
          }
          if (key.VK == iplug::kVK_F) {
            this->arrangeNodes();
            return true;
          }
          if (key.VK == iplug::kVK_C) {
            this->centerGraph();
            return true;
          }
          if (key.VK == iplug::kVK_Q) {
            this->centerNode(mInputNode);
            return true;
          }
          if (key.VK == iplug::kVK_E) {
            this->centerNode(mOutputNode);
            return true;
          }
          if (key.VK == iplug::kVK_S) {
            this->lockAudioThread();
            SortGraph::sortGraph(&mNodes, mInputNode, mOutputNode);
            this->unlockAudioThread();
            return true;
          }
        }
        return false;
      });

      mBackground = new GraphBackground(mBus, mGraphics, [&](float x, float y, float scale) {
        this->onViewPortChange(x, y, scale);
      });
      mGraphics->AttachControl(mBackground);

      for (int n = 0; n < mNodes.size(); n++) {
        mNodes[n]->setupUi(mGraphics);
      }
      mInputNode->setupUi(mGraphics);
      mOutputNode->setupUi(mGraphics);

      mCableLayer = new CableLayer(mBus, mGraphics, &mNodes, mOutputNode, mInputNode);
      mCableLayer->SetRenderPriority(10);
      mGraphics->AttachControl(mCableLayer);

      mSideBar = new SideBar(mBus, mGraphics);
      mGraphics->AttachControl(mSideBar);

      scaleUi();
#ifndef NDEBUG
      testadd();
#endif
    }

    /**
     * Updates the scale in the background layer and scales the UI according to
     * mWindowWidth, mWindowHeight, mWindowScale
     */
    void scaleUi() const {
      if (mWindowWidth != 0 && mWindowHeight != 0 && mWindowScale != 0 && mGraphics != nullptr) {
        mBackground->mScale = mWindowScale;
        mGraphics->Resize(mWindowWidth, mWindowHeight, mWindowScale);
      }
    }

    void cleanupUi() {
      SoundWoofer::instance().clearAsyncQueue();
      mWindowWidth = mGraphics->Width();
      mWindowHeight = mGraphics->Height();
      mWindowScale = mGraphics->GetDrawScale();
      for (int n = 0; n < mNodes.size(); n++) {
        mNodes[n]->cleanupUi(mGraphics);
      }

      mGraphics->RemoveControl(mSideBar);
      mSideBar = nullptr;

      mGraphics->RemoveControl(mBackground);
      mBackground = nullptr;

      mGraphics->RemoveControl(mCableLayer);
      mCableLayer = nullptr;

      mInputNode->cleanupUi(mGraphics);
      mOutputNode->cleanupUi(mGraphics);
      mGraphics->RemoveAllControls();
      mGraphics = nullptr;
    }

    /**
     * Called via a callback from the background to move around all the nodes
     * creating the illusion of a viewport
     */
    void onViewPortChange(const float dX = 0, const float dY = 0, float scale = 1) const {
      for (int i = 0; i < mNodes.size(); i++) {
        mNodes[i]->mUi->translate(dX, dY);
      }
      mOutputNode->mUi->translate(dX, dY);
      mInputNode->mUi->translate(dX, dY);
      // WDBGMSG("x %f y %f s %f\n", x, y, scale);
    }

    /**
     * Centers the viewport around a specific node
     */
    void centerNode(Node* node) const {
      IRECT center = mGraphics->GetBounds().GetScaledAboutCentre(0);
      center.L -= node->shared.X;
      center.T -= node->shared.Y;
      onViewPortChange(center.L, center.T);
    }

    /**
     * Averages all node positions and moves the viewport to that point
     * Bound to the C key
     */
    void centerGraph() const {
      Coord2D avg{ 0, 0 };
      const int count = mNodes.size();
      for (int i = 0; i < count; i++) {
        const Node* n = mNodes[i];
        avg.x += n->shared.X;
        avg.y += n->shared.Y;
      }
      float countf = count + 2;
      avg.x += mInputNode->shared.X + mOutputNode->shared.X;
      avg.y += mInputNode->shared.Y + mOutputNode->shared.Y;
      // We want that point to be in the center of the screen
      const IRECT center = mGraphics->GetBounds().GetScaledAboutCentre(0);
      avg.x = center.L - avg.x / countf;
      avg.y = center.T - avg.y / countf;
      onViewPortChange(avg.x, avg.y);
    }

    /**
     * Used to add nodes and pause the audio thread
     */
    void addNode(Node* node, Node* pInput = nullptr, const float x = 0, const float y = 0, const int outputIndex = 0, const int inputIndex = 0, Node* clone = nullptr) {
      node->shared.X = x;
      node->shared.Y = y;
      node->setup(mBus, mSampleRate, mMaxBlockSize);
      if (clone != nullptr) {
        node->copyState(clone);
      }
      mParamManager->claimNode(node);
      node->setupUi(mGraphics);
      if (pInput != nullptr) {
        node->connectInput(pInput->shared.socketsOut[outputIndex], inputIndex);
      }
      // Allocating the node is thread safe, but not the node list itself
      lockAudioThread();
      mNodes.add(node);
      SortGraph::sortGraph(&mNodes, mInputNode, mOutputNode);
      // mMaxBlockSize = hasFeedBackNode() ? MIN_BLOCK_SIZE : MAX_BUFFER;
      unlockAudioThread();
    }

    /**
     * Will re route the connections the node provided
     * Only takes care of the first input and first output
     */
    void byPassConnection(Node* node) const {
      if (node->shared.inputCount > 0 && node->shared.outputCount > 0) {
        NodeSocket* prevSock = node->shared.socketsIn[0];
        NodeSocket* nextSock = node->shared.socketsOut[0];
        if (prevSock != nullptr && prevSock->mConnectedTo[0] != nullptr && nextSock != nullptr) {
          for (int i = 0; i < MAX_SOCKET_CONNECTIONS; i++) {
            if (nextSock->mConnectedTo[i] != nullptr) {
              MessageBus::fireEvent<SocketConnectRequest>(mBus,
                MessageBus::SocketRedirectConnection,
                SocketConnectRequest{
                  prevSock->mConnectedTo[0],
                  nextSock->mConnectedTo[i]
                }
              );
            }
          }
          MessageBus::fireEvent<Node*>(mBus, MessageBus::NodeDisconnectAll, node);
        }
      }
    }

    void spliceInCombine(Node* node) {
      if (node->shared.inputCount > 0 && node->shared.outputCount > 0) {
        NodeSocket* inSock = node->shared.socketsIn[0];
        NodeSocket* outSock = node->shared.socketsOut[0];
        NodeSocket* source = inSock->mConnectedTo[0];
        NodeSocket* target = outSock->mConnectedTo[0];
        if (target == nullptr || source == nullptr) {
          return;
        }
        Node* combine = NodeList::createNode("CombineNode");
        addNode(combine, nullptr, node->shared.X, node->shared.Y);

        for (int i = 0; i < MAX_SOCKET_CONNECTIONS; i++) {
          if (outSock->mConnectedTo[i] != nullptr) {
            combine->shared.socketsOut[0]->connect(outSock->mConnectedTo[i]);
          }
        }
        combine->shared.socketsIn[0]->connect(outSock);
        combine->shared.socketsIn[1]->connect(source);
        combine->mUi->mDragging = true;
        mGraphics->SetCapturedControl(combine->mUi);
      }
    }

    /**
     * Will try to tidy up the node graph, bound to the F key
     */
    void arrangeNodes() const {
      FormatGraph::resetBranchPos(mInputNode);
      FormatGraph::arrangeBranch(mInputNode, Coord2D{ mInputNode->shared.Y, mInputNode->shared.X });
      centerGraph();
    }
#endif

    void removeAllNodes() {
      while (mNodes.size()) {
        removeNode(0);
      }
    }

    /**
     * Removes the node and pauses the audio thread
     * Can also bridge the connection if possible
     */
    void removeNode(Node* node, const bool reconnect = false) {
      if (node == mInputNode || node == mOutputNode) { return; }
      lockAudioThread();
#ifndef GUITARD_HEADLESS
      if (reconnect) {
        /**
         * Since the cleanup will sever all connections to a node, it will have to be done before the
         * connection is bridged, or else the bridged connection will be severed again
         */
        byPassConnection(node);
      }
      node->cleanupUi(mGraphics);
#endif
      mParamManager->releaseNode(node);
      node->cleanUp();
      mNodes.remove(node);
      delete node;
#ifndef GUITARD_HEADLESS
      SortGraph::sortGraph(&mNodes, mInputNode, mOutputNode);
#endif
      // mMaxBlockSize = hasFeedBackNode() ? MIN_BLOCK_SIZE : MAX_BUFFER;
      unlockAudioThread();
    }

    void removeNode(const int index) {
      removeNode(mNodes[index]);
    }



#ifndef GUITARD_HEADLESS
    void serialize(WDL_String& serialized) {
      nlohmann::json json;
      serialize(json);
      try {
        serialized.Set(json.dump().c_str());
      }
      catch (...) {
        assert(false); // Failed to dump json
      }
    }

    void serialize(nlohmann::json& json) {
      try {
        json = {
          { "version", PLUG_VERSION_HEX },
        };
        if (mGraphics != nullptr) {
          mWindowWidth = mGraphics->Width();
          mWindowHeight = mGraphics->Height();
          mWindowScale = mGraphics->GetDrawScale();
        }
        json["scale"] = mWindowScale;
        json["width"] = mWindowWidth;
        json["height"] = mWindowHeight;
        json["maxBlockSize"] = mMaxBlockSize;
        Serializer::serialize(json, &mNodes, mInputNode, mOutputNode);
      }
      catch (...) {
        assert(false); // Failed to serialize json
      }
    }
#endif

    void deserialize(const char* data) {
      try {
        nlohmann::json json = nlohmann::json::parse(data);
        deserialize(json);
      }
      catch (...) {
        return;
        // assert(false); // Failed to parse JSON
      }
    }

    void deserialize(nlohmann::json& json) {
      try {
        removeAllNodes();
        if (json.contains("scale")) {
          // mWindowScale = json["scale"];
          // mWindowWidth = json["width"]; // Probably no point in changing the window size since it's confusing
          // mWindowHeight = json["height"];
        }
        if (json.contains("maxBlockSize")) {
          mMaxBlockSize = json["maxBlockSize"];
        }
        lockAudioThread();
        Serializer::deserialize(json, &mNodes, mOutputNode, mInputNode, mSampleRate, mMaxBlockSize, mParamManager, mBus);
#ifndef GUITARD_HEADLESS
        if (mGraphics != nullptr && mGraphics->WindowIsOpen()) {
          for (int i = 0; i < mNodes.size(); i++) {
            mNodes[i]->setupUi(mGraphics);
          }
          scaleUi();
        }
#endif
        //if (json.contains("maxBlockSize")) {
        //  setBlockSize(json["maxBlockSize"]);
        //}
        unlockAudioThread();
      }
      catch (...) {
        // assert(false); // To load graph with json
      }
    }

  private:
#ifndef GUITARD_HEADLESS
    /**
     * Test Setups
     */
    void testadd() {
      return;
      // formatTest();
      Node* test = NodeList::createNode("CabLibNode");
      addNode(test, nullptr, 0, 500);
      // mOutputNode->connectInput(test->shared.socketsOut[0]);
    }

    void formatTest() {
      /**
       *                            ------------
       *                            |          |
       *                --> test2 --|          |--> test5 --
       *                |           |          |           |
       *  in -> test1 --|           --> test4 --           |--> test7 --> out
       *                |                                  |
       *                --> test3 ----> test6 --------------
       */
      Node* test1 = NodeList::createNode("StereoToolNode");
      addNode(test1, mInputNode, 200, 0);
      Node* test2 = NodeList::createNode("StereoToolNode");
      addNode(test2, test1, 400, -100);
      Node* test3 = NodeList::createNode("StereoToolNode");
      addNode(test3, test1, 400, +100);
      Node* test4 = NodeList::createNode("StereoToolNode");
      addNode(test4, test2, 600, 0);
      Node* test5 = NodeList::createNode("CombineNode");
      addNode(test5, test2, 800, +100);
      test5->connectInput(test4->shared.socketsOut[0], 1);
      Node* test6 = NodeList::createNode("StereoToolNode");
      addNode(test6, test3, 400, +100);
      Node* test7 = NodeList::createNode("CombineNode");
      addNode(test7, test5, 1000, 0);
      test7->connectInput(test6->shared.socketsOut[0], 1);
      mOutputNode->shared.socketsIn[0]->disconnectAll();
      mOutputNode->connectInput(test7->shared.socketsOut[0]);
    }
#endif
  };
}
