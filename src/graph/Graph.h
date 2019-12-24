#pragma once
#include <chrono>
#include "mutex.h"
#include "thirdparty/json.hpp"
#include "IPlugConstants.h"
#include "src/misc/MessageBus.h"
#include "src/nodes/io/InputNode.h"
#include "src/nodes/io/OutputNode.h"
#include "src/ui/GraphBackground.h"
#include "src/graph/Serializer.h"
#include "src/parameter/ParameterManager.h"
#include "src/ui/CableLayer.h"
#include "src/ui/gallery/NodeGallery.h"
#include "src/misc/HistoryStack.h"
#include "src/nodes/envelope/EnvelopeNode.h"

class Graph {
  MessageBus::Bus* mBus = nullptr;
  MessageBus::Subscription<Node*> mNodeDelSub;
  MessageBus::Subscription<Node*> mNodeBypassEvent;
  MessageBus::Subscription<Node*> mNodeCloneEvent;
  MessageBus::Subscription<NodeList::NodeInfo> mNodeAddEvent;
  MessageBus::Subscription<bool> mAwaitAudioMutexEvent;
  MessageBus::Subscription<bool> mPushUndoState;
  MessageBus::Subscription<bool> mPopUndoState;
  MessageBus::Subscription<GraphStats**> mReturnStats;
  MessageBus::Subscription<AutomationAttachRequest> mAutomationRequest;

  iplug::igraphics::IGraphics* mGraphics = nullptr;
  /** Holds all the nodes in the processing graph */
  WDL_PtrList<Node> mNodes;
  WDL_Mutex mIsProcessing;
  /** Dummy nodes to get the audio blocks in and out of the graph */
  InputNode* mInputNode;
  OutputNode* mOutputNode;

  /**
   * This is the channel count to be used internally
   * All nodes will allocate buffers to according to this
   * Using anything besides stereo will cause problems with the faust DSP code
   */
  int mChannelCount = 0;

  int mInPutChannelCount = 0;

  int mSampleRate = 0;

  int mMaxBlockSize = MAX_BUFFER;

  /** Control elements */
  GraphBackground* mBackground = nullptr;
  CableLayer* mCableLayer = nullptr;
  NodeGallery* mNodeGallery = nullptr;

  HistoryStack mHistoryStack;

  GraphStats mStats;

  /** Editor window properties */
  int mWindowWidth = 0;
  int mWindowHeight = 0;
  float mWindowScale = 0;

  /** Used to slice the dsp block in smaller slices */
  sample** mSliceBuffer[2] = { nullptr };

public:
  ParameterManager mParamManager;


  explicit Graph(MessageBus::Bus* pBus) : mParamManager(pBus) {
    mBus = pBus;
    
    mInputNode = new InputNode(mBus);
    mOutputNode = new OutputNode(mBus);
    mOutputNode->connectInput(mInputNode->shared.socketsOut[0]);

    // output->connectInput(input->outSockets.Get(0));
    mNodeDelSub.subscribe(mBus, MessageBus::NodeDeleted, [&](Node* param) {
      MessageBus::fireEvent(mBus, MessageBus::PushUndoState, false);
      this->removeNode(param, true);
    });

    mNodeBypassEvent.subscribe(mBus, MessageBus::BypassNodeConnection, [&](Node* param) {
      MessageBus::fireEvent(mBus, MessageBus::PushUndoState, false);
      this->byPassConnection(param);
    });

    mNodeAddEvent.subscribe(mBus, MessageBus::NodeAdd, [&](const NodeList::NodeInfo info) {
      MessageBus::fireEvent(mBus, MessageBus::PushUndoState, false);
      this->addNode(info.constructor(), nullptr,300, 300);
    });

    mNodeCloneEvent.subscribe(mBus, MessageBus::CloneNode, [&](Node* node) {
      Node* clone = NodeList::createNode(node->shared.type);
      if (clone != nullptr) {
        this->addNode(clone, nullptr, node->shared.X, node->shared.Y, 0, 0, node);
        clone->mUi->mDragging = true;
        mGraphics->SetCapturedControl(clone->mUi);
      }
    });

    // This might not even make sense
    mAwaitAudioMutexEvent.subscribe(mBus, MessageBus::AwaitAudioMutex, [&](bool) {
      //WDL_MutexLock lock(&isProcessing);
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

    mAutomationRequest.subscribe(mBus, MessageBus::AttachAutomation, [&](AutomationAttachRequest r) {
      MessageBus::fireEvent(mBus, MessageBus::PushUndoState, false);
      WDL_PtrList<Node>& n = this->mNodes;
      for (int i = 0; i < n.GetSize(); i++) {
        Node* node = n.Get(i);
        if (node == nullptr) { continue; }
        for (int p = 0; p < node->shared.parameterCount; p++) {
          if (node->shared.parameters[p]->control == r.targetControl) {
            if (node != r.automationNode) {
              // Don't allow automation on self
              node->attachAutomation(r.automationNode, p);
            }
          }
        }
      }
    });
  }

  void testadd() {
    // formatTest();
    Node* test = NodeList::createNode("CabLibNode");
    addNode(test, nullptr, 0, 500);
    // mOutputNode->connectInput(test->shared.socketsOut[0]);
  }

  ~Graph() {
    removeAllNodes();
    // TODOG get rid of all the things
  }

  void OnReset(const int pSampleRate, const int pOutputChannels = 2, const int pInputChannels = 2) {
    if (pSampleRate != mSampleRate || pOutputChannels != mChannelCount || pInputChannels != mInPutChannelCount) {
      WDL_MutexLock lock(&mIsProcessing);
      mSampleRate = pSampleRate;
      resizeSliceBuffer(pOutputChannels);
      mChannelCount = pOutputChannels;
      mInPutChannelCount = pInputChannels;
      mInputNode->setInputChannels(pInputChannels);
      mInputNode->OnReset(pSampleRate, pOutputChannels);
      mOutputNode->OnReset(pSampleRate, pOutputChannels);
      for (int i = 0; i < mNodes.GetSize(); i++) {
        mNodes.Get(i)->OnReset(pSampleRate, pOutputChannels);
      }
    }
    else {
      /**
       * If nothing has changed we'll assume a transport
       */
      mInputNode->OnTransport();
      mOutputNode->OnTransport();
      for (int i = 0; i < mNodes.GetSize(); i++) {
        mNodes.Get(i)->OnTransport();
      }
    }
  }

  void resizeSliceBuffer(const int channelCount) {
    if (mSliceBuffer[0] != nullptr) {
      for (int c = 0; c < channelCount; c++) {
        delete mSliceBuffer[0];
        mSliceBuffer[0] = nullptr;
        delete mSliceBuffer[1];
        mSliceBuffer[1] = nullptr;
      }
    }
    mSliceBuffer[0] = new sample*[channelCount];
    mSliceBuffer[1] = new sample*[channelCount];
  }

  void ProcessBlock(iplug::sample** in, iplug::sample** out, const int nFrames) {
    if (nFrames > mMaxBlockSize) {
      /** Process the block in smaller bits since it's too large */
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

    const auto start = std::chrono::high_resolution_clock::now();
    WDL_MutexLock lock(&mIsProcessing);
    mInputNode->CopyIn(in, nFrames);

    const int nodeCount = mNodes.GetSize();
    for (int n = 0; n < nodeCount; n++) {
      mNodes.Get(n)->BlockStart();
    }

    mOutputNode->BlockStart();

    // The List is pre sorted so the attempts are only needed to catch circular dependencies and other edge cases
    const int maxAttempts = 10;
    int attempts = 0;
    while (!mOutputNode->mIsProcessed && attempts < maxAttempts) {
      for (int n = 0; n < nodeCount; n++) {
        mNodes.Get(n)->ProcessBlock(nFrames);
      }
      mOutputNode->ProcessBlock(nFrames);
      attempts++;
    }

    // This extra iteration makes sure the feedback loops get data from their previous nodes
    if (attempts < maxAttempts) {
      for (int n = 0; n < nodeCount; n++) {
        mNodes.Get(n)->ProcessBlock(nFrames);
      }
    }

    mOutputNode->CopyOut(out, nFrames);
    const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::high_resolution_clock::now() - start
    );
    mStats.executionTime = duration.count();
  }

  /**
   * The graph needs to know about the graphics context to add and remove the controls for the nodes
   * It also handles keystrokes globally
   */
  void setupUi(iplug::igraphics::IGraphics* pGraphics = nullptr) {
    if (pGraphics != nullptr && pGraphics != mGraphics) {
      WDBGMSG("Graphics context changed");
      mGraphics = pGraphics;
    }

    pGraphics->HandleMouseOver(true);
    
    mGraphics->SetKeyHandlerFunc([&](const IKeyPress & key, const bool isUp) {
      // Gets the keystrokes in the standalone app
      // TODOG figure out why this doesn't work in vst3
      if (key.S && (key.VK == kVK_Z) && !isUp) {
        MessageBus::fireEvent<bool>(this->mBus, MessageBus::PopUndoState, false);
        return true;
      }
      if ((key.VK == kVK_F) && !isUp) {
        arrangeNodes();
        return true;
      }
      if ((key.VK == kVK_C) && !isUp) {
        centerGraph();
        return true;
      }
      if ((key.VK == kVK_Q) && !isUp) {
        centerNode(mInputNode);
        return true;
      }
      if ((key.VK == kVK_E) && !isUp) {
        centerNode(mOutputNode);
        return true;
      }
      if ((key.VK == kVK_S) && !isUp) {
        sortGraph();
        return true;
      }
      return false;
    });

    mBackground = new GraphBackground(mBus, mGraphics, [&](float x, float y, float scale) {
      this->onViewPortChange(x, y, scale);
    });
    mGraphics->AttachControl(mBackground);

    for (int n = 0; n < mNodes.GetSize(); n++) {
        mNodes.Get(n)->setupUi(mGraphics);
    }
    mInputNode->setupUi(mGraphics);
    mOutputNode->setupUi(mGraphics);

    mCableLayer = new CableLayer(mBus, mGraphics, &mNodes, mOutputNode, mInputNode);
    mGraphics->AttachControl(mCableLayer);

    mNodeGallery = new NodeGallery(mBus, mGraphics);
    mGraphics->AttachControl(mNodeGallery);

    scaleUi();

    testadd();
  }

  void scaleUi() const {
    if (mWindowWidth != 0 && mWindowHeight != 0 && mWindowScale != 0 && mGraphics != nullptr) {
      mBackground->mScale = mWindowScale;
      mGraphics->Resize(mWindowWidth, mWindowHeight, mWindowScale);
    }
  }

  void cleanupUi() {
    mWindowWidth = mGraphics->Width();
    mWindowHeight = mGraphics->Height();
    mWindowScale = mGraphics->GetDrawScale();
    for (int n = 0; n < mNodes.GetSize(); n++) {
      mNodes.Get(n)->cleanupUi(mGraphics);
    }

    mGraphics->RemoveControl(mNodeGallery, true);
    mNodeGallery = nullptr;

    mGraphics->RemoveControl(mBackground, true);
    mBackground = nullptr;

    mGraphics->RemoveControl(mCableLayer, true);
    mCableLayer = nullptr;

    mInputNode->cleanupUi(mGraphics);
    mOutputNode->cleanupUi(mGraphics);

    mGraphics = nullptr;
  }

  /**
   * Called via a callback from the background to move around all the nodes
   * creating the illusion of a viewport
   */
  void onViewPortChange(const float dX = 0, const float dY = 0, float scale = 1) const {
    for (int i = 0; i < mNodes.GetSize(); i++) {
      mNodes.Get(i)->mUi->translate(dX, dY);
    }
    mOutputNode->mUi->translate(dX, dY);
    mInputNode->mUi->translate(dX, dY);
    // WDBGMSG("x %f y %f s %f\n", x, y, scale);
  }

  /** Centers the viewport around a specific node */
  void centerNode(Node* node) {
    IRECT center = mGraphics->GetBounds().GetScaledAboutCentre(0);
    center.L -= node->shared.X;
    center.T -= node->shared.Y;
    onViewPortChange(center.L, center.T);
  }

  /**
   * Averages all node positions and moves the viewport to that point
   * Bound to the C key
   */
  void centerGraph() {
    Coord2D avg{ 0, 0 };
    const int count = mNodes.GetSize();
    for (int i = 0; i < count; i++) {
      const Node* n = mNodes.Get(i);
      avg.x += n->shared.X;
      avg.y += n->shared.Y;
    }
    float countf = count + 2;
    avg.x += mInputNode->shared.X + mOutputNode->shared.X;
    avg.y += mInputNode->shared.Y + mOutputNode->shared.Y;
    // We want that point to be in the center of the screen
    IRECT center = mGraphics->GetBounds().GetScaledAboutCentre(0);
    avg.x = center.L - avg.x / countf;
    avg.y = center.T - avg.y / countf;
    onViewPortChange(avg.x, avg.y);
  }

  void layoutUi(iplug::igraphics::IGraphics* pGraphics = nullptr) {
    if (pGraphics != nullptr && pGraphics != mGraphics) {
      WDBGMSG("Graphics context changed");
      mGraphics = pGraphics;
      // TODOG find out whether the context ever changes
    }
    for (int n = 0; n < mNodes.GetSize(); n++) {
      mNodes.Get(n)->layoutChanged();
    }
  }

  /**
   * Used to add nodes and pause the audio thread
   */
  void addNode(Node* node, Node* pInput = nullptr, const float x = 0, const float y = 0, const int outputIndex = 0, const int inputIndex = 0, Node* clone = nullptr) {
    WDL_MutexLock lock(&mIsProcessing);
    node->shared.X = x;
    node->shared.Y = y;
    node->setup(mBus, mSampleRate);
    if (clone != nullptr) {
      node->copyState(clone);
    }
    mParamManager.claimNode(node);
    node->setupUi(mGraphics);
    if (pInput != nullptr) {
      node->connectInput(pInput->shared.socketsOut[outputIndex], inputIndex);
    }
    mNodes.Add(node);
    sortGraph(false);
    sortRenderStack();
    mMaxBlockSize = hasFeedBackNode() ? MIN_BLOCK_SIZE : MAX_BUFFER;
  }

  void removeAllNodes() {
    while (mNodes.GetSize()) {
      removeNode(0);
    }
  }

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

  /**
   * Removes the node and pauses the audio thread
   * Can also bridge the connection if possible
   */
  void removeNode(Node* node, const bool reconnect = false) {
    if (node == mInputNode || node == mOutputNode) { return; }
    /**
     * Since the cleanup will sever all connections to a node, it will have to be done before the
     * connection is bridged, or else the bridged connection will be severed again
     */
    WDL_MutexLock lock(&mIsProcessing);
    if (reconnect) {
      byPassConnection(node);
    }
    node->cleanupUi(mGraphics);
    mParamManager.releaseNode(node);
    node->cleanUp();
    mNodes.DeletePtr(node, true);
    sortGraph(false);
    mMaxBlockSize = hasFeedBackNode() ? MIN_BLOCK_SIZE : MAX_BUFFER;
  }

  void removeNode(const int index) {
    removeNode(mNodes.Get(index));
  }

  void serialize(nlohmann::json& json) {
    if (mGraphics != nullptr) {
      mWindowWidth = mGraphics->Width();
      mWindowHeight = mGraphics->Height();
      mWindowScale = mGraphics->GetDrawScale();
    }
    json["scale"] = mWindowScale;
    json["width"] = mWindowWidth;
    json["height"] = mWindowHeight;
    Serializer::serialize(json, mNodes, mInputNode, mOutputNode);
  }

  void deserialize(nlohmann::json& json) {
    removeAllNodes();
    if (json.contains("width")) {
      mWindowScale = json["scale"];
      mWindowWidth = json["width"];
      mWindowHeight = json["height"];
    }
    WDL_MutexLock lock(&mIsProcessing);
    Serializer::deserialize(json, mNodes, mOutputNode, mInputNode, mSampleRate, &mParamManager, mBus);
    if (mGraphics != nullptr && mGraphics->WindowIsOpen()) {
      for (int i = 0; i < mNodes.GetSize(); i++) {
        mNodes.Get(i)->setupUi(mGraphics);
      }
      sortRenderStack();
      scaleUi();
    }
  }

private:
  /**
   * This just removes the overlay IControls and adds them again to
   * keep them on top of the layer stack
   */
  void sortRenderStack() const {
    mGraphics->MoveOnTop(mCableLayer);
    mGraphics->MoveOnTop(mNodeGallery);
    if (mNodeGallery != nullptr) {
      mGraphics->MoveOnTop(mNodeGallery->mScrollview);
    }
  }

  /**
   * Will try to tidy up the node graph, bound to the F key
   */
  void arrangeNodes() {
    resetBranchPos(mInputNode);
    arrangeBranch(mInputNode, Coord2D{ mInputNode->shared.Y, mInputNode->shared.X });
    centerGraph();
  }

  /**
   * Recursively resets all the positions of nodes to (0, 0)
   */
  static void resetBranchPos(Node* node) {
    if (node == nullptr || node->shared.type == "FeedbackNode") { return; }
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
   * Recursively sorts nodes
   */
  Coord2D arrangeBranch(Node* node, Coord2D pos) {
    if (node == nullptr || node->shared.type == "FeedbackNode") {
      return pos;
    }
    const float halfWidth = node->shared.width * 0.5;
    const float halfHeight = node->shared.height * 0.5;
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
    return Coord2D { nextX, pos.y };
  }

  bool hasFeedBackNode() const {
    for (int i = 0; i < mNodes.GetSize(); i++) {
      if (mNodes.Get(i)->shared.type == "FeedbackNode") {
        return true;
      }
    }
    return false;
  }

  /**
   * Does some sorting on the mNodes list so the graph can be computed with fewer attempts
   */
  void sortGraph(bool lock = true) {
    WDL_PtrList<Node> sorted;
    for (int i = 0; i < MAX_SOCKET_CONNECTIONS; i++) {
      // Put in the nodes which directly follow the input node
      NodeSocket* con = mInputNode->shared.socketsOut[0]->mConnectedTo[i];
      if (con != nullptr) {
        sorted.Add(con->mParentNode);
      }
    }

    // Arbitrary depth
    for (int tries = 0; tries < 100; tries++) {
      for (int i = 0; i < sorted.GetSize(); i++) {
        Node* node = sorted.Get(i);
        for (int out = 0; out < node->shared.outputCount; out++) {
          NodeSocket* outSocket = node->shared.socketsOut[out];
          if (outSocket == nullptr) { continue; }
          for (int next = 0; next < MAX_SOCKET_CONNECTIONS; next++) {
            NodeSocket* nextSocket = outSocket->mConnectedTo[next];
            if (nextSocket == nullptr) { continue; }
            Node* nextNode = nextSocket->mParentNode;
            // Don't want to add duplicates or the output node
            if (sorted.Find(nextNode) != -1) { continue; }
            sorted.Add(nextNode);
          }
        }
      }
    }

    // Add in all the nodes which might not be connected or were missed because of the depth limit
    for (int i = 0; i < mNodes.GetSize(); i++) {
      Node* nextNode = mNodes.Get(i);
      if (sorted.Find(nextNode) != -1) { continue; }
      sorted.Add(nextNode);
    }

    WDL_MutexLock* mutex;
    if (lock) {
      mutex = new WDL_MutexLock(&mIsProcessing);
    }
    mNodes.Empty(false);
    for (int i = 0; i < sorted.GetSize(); i++) {
      Node* n = sorted.Get(i);
      if (n == mOutputNode) { continue; }
      mNodes.Add(n);
    }
    if (lock) {
      delete mutex;
    }
  }

  /**
   * Test Setups
   */

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

};
