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

  int mChannelCount = 2;
  int mSampleRate = 44101;

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
    return;
    Node* test = NodeList::createNode("EnvelopeNode");
    addNode(test, mInputNode, 0, 500, 300);
    // mOutputNode->connectInput(test->shared.socketsOut[0]);
  }

  ~Graph() {
    removeAllNodes();
    // TODOG get rid of all the things
  }

  void OnReset(const int pSampleRate, const int pChannels = 2) {
    if (pSampleRate > 0 && pChannels > 0) {
      WDL_MutexLock lock(&mIsProcessing);
      mSampleRate = pSampleRate;
      resizeSliceBuffer(pChannels);
      mChannelCount = pChannels;
      mInputNode->OnReset(pSampleRate, pChannels);
      mOutputNode->OnReset(pSampleRate, pChannels);
      for (int i = 0; i < mNodes.GetSize(); i++) {
        mNodes.Get(i)->OnReset(pSampleRate, pChannels);
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
    if (nFrames > MAX_BUFFER) {
      /** Process the block in smaller bits since it's too large */
      const int overhang = nFrames % MAX_BUFFER;
      int s = 0;
      while (true) {
        for (int c = 0; c < mChannelCount; c++) {
          mSliceBuffer[0][c] = &in[c][s];
          mSliceBuffer[1][c] = &out[c][s];
        }
        s += MAX_BUFFER;
        if (s <= nFrames) {
          ProcessBlock(mSliceBuffer[0], mSliceBuffer[1], MAX_BUFFER);
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

    // TODOG multiple passes to ensure all the nodes are computed is super dumb
    int attempts = 0;
    while (!mOutputNode->mIsProcessed && attempts < 10) {
      for (int n = 0; n < nodeCount; n++) {
        mNodes.Get(n)->ProcessBlock(nFrames);
      }
      mOutputNode->ProcessBlock(nFrames);
      attempts++;
    }
    // HACK only used for the feedback node to get the latest data in the graph
    for (int n = 0; n < nodeCount; n++) {
      mNodes.Get(n)->ProcessBlock(nFrames);
    }

    mOutputNode->CopyOut(out, nFrames);
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

    pGraphics->HandleMouseOver(true);
    
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
   * Used to add nodes and push a state to the history stack
   */
  void addNode(Node* node, Node* pInput = nullptr, const int index = 0, const float x = 0, const float y = 0) {
    WDL_MutexLock lock(&mIsProcessing);
    node->shared.X = x;
    node->shared.Y = y;
    node->setup(mBus, mSampleRate);
    mParamManager.claimNode(node);
    node->setupUi(mGraphics);
    if (pInput != nullptr) {
      node->connectInput(pInput->shared.socketsOut[index]);
    }
    mNodes.Add(node);
    sortRenderStack();
  }

  void removeAllNodes() {
    while (mNodes.GetSize()) {
      removeNode(0);
    }
  }

  void removeNode(Node* node, const bool reconnect = false) {
    if (node == mInputNode || node == mOutputNode) { return; }
    WDL_MutexLock lock(&mIsProcessing);
    if (reconnect && node->shared.inputCount > 0 && node->shared.outputCount > 0) {
      NodeSocket* prevSock = node->shared.socketsIn[0];
      NodeSocket* nextSock = node->shared.socketsOut[0];
      if (prevSock != nullptr && prevSock->mConnectedTo && nextSock != nullptr) {
        MessageBus::fireEvent<SocketConnectRequest>(
          mBus,
          MessageBus::SocketRedirectConnection,
          SocketConnectRequest {
            nextSock,
            prevSock->mConnectedTo
          }
        );
      }
    }
    node->cleanupUi(mGraphics);
    node->cleanUp();
    mParamManager.releaseNode(node);
    mNodes.DeletePtr(node, true);
    mNodes.Compact();
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
    if (json.find("width") != json.end()) {
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
