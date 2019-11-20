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

class Graph {
  MessageBus::Bus* mBus = nullptr;
  MessageBus::Subscription<Node*> mNodeDelSub;
  MessageBus::Subscription<NodeList::NodeInfo> mNodeAddEvent;
  MessageBus::Subscription<bool> mAwaitAudioMutexEvent;
  MessageBus::Subscription<bool> mPushUndoState;
  MessageBus::Subscription<bool> mPopUndoState;
  MessageBus::Subscription<GraphStats**> mReturnStats;

  iplug::igraphics::IGraphics* mGraphics = nullptr;
  /** Holds all the nodes in the processing graph */
  WDL_PtrList<Node> nodes;
  WDL_Mutex isProcessing;
  /** Dummy nodes to get the audio blocks in and out of the graph */
  InputNode* inputNode;
  OutputNode* outputNode;

  int channelCount = 2;
  int sampleRate = 44101;

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

public:
  ParameterManager paramManager;


  explicit Graph(MessageBus::Bus* pBus) : paramManager(pBus) {
    mBus = pBus;
    
    inputNode = new InputNode(mBus);
    outputNode = new OutputNode(mBus);
    outputNode->connectInput(inputNode->mSocketsOut.Get(0));

    // output->connectInput(input->outSockets.Get(0));
    mNodeDelSub.subscribe(mBus, MessageBus::NodeDeleted, [&](Node* param) {
      MessageBus::fireEvent(mBus, MessageBus::PushUndoState, false);
      this->removeNode(param, true);
    });

    mNodeAddEvent.subscribe(mBus, MessageBus::NodeAdd, [&](const NodeList::NodeInfo info) {
      MessageBus::fireEvent(mBus, MessageBus::PushUndoState, false);
      this->addNode(info.constructor(), nullptr, 0, 300, 300);
    });

    // This might not even make sense
    mAwaitAudioMutexEvent.subscribe(mBus, MessageBus::AwaitAudioMutex, [&](bool) {
      //WDL_MutexLock lock(&isProcessing);
    });

    mPushUndoState.subscribe(mBus, MessageBus::PushUndoState, [&](bool) {
      WDBGMSG("PushState");
      this->serialize(*(mHistoryStack.pushState()));
    });

    mPopUndoState.subscribe(mBus, MessageBus::PopUndoState, [&](bool redo) {
      nlohmann::json* state = mHistoryStack.popState(redo);
      if (state != nullptr) {
        WDBGMSG("PopState");
        this->deserialize(*state);
      }
    });

    mReturnStats.subscribe(mBus, MessageBus::GetGraphStats, [&](GraphStats** stats) {
      *stats = &mStats;
    });
  }

  void testadd() {
    Node* test = NodeList::createNode("BitCrusherNode");
    addNode(test, inputNode, 0, 500, 300);
    outputNode->connectInput(test->mSocketsOut.Get(0));
  }

  ~Graph() {
    // TODOG get rid of all the things
  }

  void OnReset(const int pSampleRate, const int pChannels = 2) {
    if (pSampleRate > 0 && pChannels > 0) {
      WDL_MutexLock lock(&isProcessing);
      sampleRate = pSampleRate;
      channelCount = pChannels;
      inputNode->OnReset(pSampleRate, pChannels);
      outputNode->OnReset(pSampleRate, pChannels);
      for (int i = 0; i < nodes.GetSize(); i++) {
        nodes.Get(i)->OnReset(pSampleRate, pChannels);
      }
    }
  }

  void ProcessBlock(iplug::sample** in, iplug::sample** out, const int nFrames) {
    const auto start = std::chrono::high_resolution_clock::now();
    WDL_MutexLock lock(&isProcessing);
    if (nFrames > MAX_BUFFER) {
      // TODOG process this in smaller chunks, should be a simple for loop
      outputNode->CopyOut(out, nFrames);
      return;
    }
    inputNode->CopyIn(in, nFrames);

    const int nodeCount = nodes.GetSize();
    for (int n = 0; n < nodeCount; n++) {
      nodes.Get(n)->BlockStart();
    }

    outputNode->BlockStart();

    // TODOG multiple passes to ensure all the nodes are computed is super dumb
    int attempts = 0;
    while (!outputNode->mIsProcessed && attempts < 10) {
      for (int n = 0; n < nodeCount; n++) {
        nodes.Get(n)->ProcessBlock(nFrames);
      }
      outputNode->ProcessBlock(nFrames);
      attempts++;
    }

    outputNode->CopyOut(out, nFrames);
    const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::high_resolution_clock::now() - start
    );
    mStats.executionTime = duration.count();
  }

  /** The graph needs to know about the graphics context to add and remove the controls for the nodes */
  void setupUi(iplug::igraphics::IGraphics* pGraphics = nullptr) {
    if (pGraphics != nullptr && pGraphics != mGraphics) {
      WDBGMSG("Graphics context changed");
      mGraphics = pGraphics;
    }

    
    mGraphics->SetKeyHandlerFunc([&](const IKeyPress & key, const bool isUp) {
      if (key.C && key.VK == kVK_Z && !isUp) {
        MessageBus::fireEvent<bool>(this->mBus, MessageBus::PopUndoState, false);
        return true;
      }
      return false;
    });

    mBackground = new GraphBackground(mBus, mGraphics, [&](float x, float y, float scale) {
      this->onViewPortChange(x, y, scale);
    });
    mGraphics->AttachControl(mBackground);

    for (int n = 0; n < nodes.GetSize(); n++) {
        nodes.Get(n)->setupUi(mGraphics);
    }
    inputNode->setupUi(mGraphics);
    outputNode->setupUi(mGraphics);

    mCableLayer = new CableLayer(mBus, mGraphics, &nodes, outputNode, inputNode);
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
    for (int n = 0; n < nodes.GetSize(); n++) {
      nodes.Get(n)->cleanupUi(mGraphics);
    }

    mGraphics->RemoveControl(mNodeGallery, true);
    mNodeGallery = nullptr;

    mGraphics->RemoveControl(mBackground, true);
    mBackground = nullptr;

    mGraphics->RemoveControl(mCableLayer, true);
    mCableLayer = nullptr;

    inputNode->cleanupUi(mGraphics);
    outputNode->cleanupUi(mGraphics);

    mGraphics = nullptr;
  }

  /**
   * Called via a callback from the background to move around all the nodes
   * creating the illusion of a viewport
   */
  void onViewPortChange(const float dX = 0, const float dY = 0, float scale = 1) const {
    for (int i = 0; i < nodes.GetSize(); i++) {
      nodes.Get(i)->mUi->translate(dX, dY);
    }
    outputNode->mUi->translate(dX, dY);
    inputNode->mUi->translate(dX, dY);
    // WDBGMSG("x %f y %f s %f\n", x, y, scale);
  }

  void layoutUi(iplug::igraphics::IGraphics* pGraphics = nullptr) {
    if (pGraphics != nullptr && pGraphics != mGraphics) {
      WDBGMSG("Graphics context changed");
      mGraphics = pGraphics;
      // TODOG find out whether the context ever changes
    }
    for (int n = 0; n < nodes.GetSize(); n++) {
      nodes.Get(n)->layoutChanged();
    }
  }

  /**
   * Used to add nodes and push a state to the history stack
   */
  void addNode(Node* node, Node* pInput = nullptr, const int index = 0, const float x = 0, const float y = 0) {
    WDL_MutexLock lock(&isProcessing);
    node->mX = x;
    node->mY = y;
    node->setup(mBus, sampleRate);
    paramManager.claimNode(node);
    node->setupUi(mGraphics);
    if (pInput != nullptr) {
      node->connectInput(pInput->mSocketsOut.Get(index));
    }
    nodes.Add(node);
    sortRenderStack();
  }

  void removeAllNodes() {
    while (nodes.GetSize()) {
      removeNode(0);
    }
  }

  void removeNode(Node* node, const bool reconnect = false) {
    if (node == inputNode || node == outputNode) { return; }
    WDL_MutexLock lock(&isProcessing);
    if (reconnect) {
      NodeSocket* prevSock = node->mSocketsIn.Get(0);
      NodeSocket* nextSock = node->mSocketsOut.Get(0);
      if (prevSock != nullptr && prevSock->connectedTo && nextSock != nullptr) {
        MessageBus::fireEvent<SocketConnectRequest>(
          mBus,
          MessageBus::SocketRedirectConnection,
          SocketConnectRequest {
            nextSock,
            prevSock->connectedTo
          }
        );
      }
    }
    node->cleanupUi(mGraphics);
    paramManager.releaseNode(node);
    nodes.DeletePtr(node, true);
    nodes.Compact();
  }

  void removeNode(const int index) {
    removeNode(nodes.Get(index));
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
    Serializer::serialize(json, nodes, inputNode, outputNode);
  }

  void deserialize(nlohmann::json& json) {
    removeAllNodes();
    if (json.find("width") != json.end()) {
      mWindowScale = json["scale"];
      mWindowWidth = json["width"];
      mWindowHeight = json["height"];
    }
    WDL_MutexLock lock(&isProcessing);
    Serializer::deserialize(json, nodes, outputNode, inputNode, sampleRate, &paramManager, mBus);
    if (mGraphics != nullptr && mGraphics->WindowIsOpen()) {
      for (int i = 0; i < nodes.GetSize(); i++) {
        nodes.Get(i)->setupUi(mGraphics);
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
    if (mCableLayer != nullptr) {
      mGraphics->RemoveControl(mCableLayer);
      mGraphics->AttachControl(mCableLayer);
    }
    if (mNodeGallery != nullptr) {
      mGraphics->RemoveControl(mNodeGallery);
      mGraphics->AttachControl(mNodeGallery);
    }
  }

};
