#pragma once
#include <chrono>
#include "mutex.h"
#include "thirdparty/json.hpp"
#include "IPlugConstants.h"
#include "src/misc/constants.h"
#include "src/misc/MessageBus.h"
#include "src/nodes/io/InputNode.h"
#include "src/nodes/io/OutputNode.h"
#include "src/ui/GraphBackground.h"
#include "src/graph/Serializer.h"
#include "src/parameter/ParameterManager.h"
#include "src/ui/CableLayer.h"
#include "src/ui/gallery/NodeGallery.h"
#include "src/misc/HistoryStack.h"
#include "GraphStats.h"

class Graph {
  MessageBus::Subscription<Node*> mNodeDelSub;
  MessageBus::Subscription<NodeList::NodeInfo> mNodeAddEvent;
  MessageBus::Subscription<bool> mAwaitAudioMutexEvent;
  MessageBus::Subscription<bool> mPushUndoState;
  MessageBus::Subscription<bool> mPopUndoState;
  MessageBus::Subscription<GraphStats**> mReturnStats;

  iplug::igraphics::IGraphics* graphics;
  /** Holds all the nodes in the processing graph */
  WDL_PtrList<Node> nodes;
  WDL_Mutex isProcessing;
  /** Dummy nodes to get the audio blocks in and out of the graph */
  InputNode* inputNode;
  OutputNode* outputNode;

  int channelCount;
  int sampleRate;

  GraphBackground* background;

  CableLayer* cableLayer;

  NodeGallery* nodeGallery;

  GraphStats mStats;

  int width;
  int height;
  float scale;

public:
  ParameterManager paramManager;


  Graph() {
    graphics = nullptr;
    background = nullptr;
    nodeGallery = nullptr;
    cableLayer = nullptr;

    width = height = scale = 0;

    /** Odd number to figure out if the DAW hasn't reported a samplerate */
    sampleRate = 44101;
    channelCount = 2;

    inputNode = new InputNode();
    outputNode = new OutputNode();
    outputNode->connectInput(inputNode->outSockets.Get(0));

    // output->connectInput(input->outSockets.Get(0));
    mNodeDelSub.subscribe(MessageBus::NodeDeleted, [&](Node* param) {
      MessageBus::fireEvent(MessageBus::PushUndoState, false);
      this->removeNode(param, true);
    });
    mNodeAddEvent.subscribe(MessageBus::NodeAdd, [&](NodeList::NodeInfo info) {
      MessageBus::fireEvent(MessageBus::PushUndoState, false);
      this->addNode(info.constructor(), nullptr, 0, 300, 300);
    });

    // This might not even make sense
    mAwaitAudioMutexEvent.subscribe(MessageBus::AwaitAudioMutex, [&](bool) {
      //WDL_MutexLock lock(&isProcessing);
    });

    mPushUndoState.subscribe(MessageBus::PushUndoState, [&](bool) {
      WDBGMSG("PushState");
      this->serialize(*HistoryStack::PushState());
    });

    mPopUndoState.subscribe(MessageBus::PopUndoState, [&](bool redo) {
      nlohmann::json* state = HistoryStack::PopState(redo);
      if (state != nullptr) {
        WDBGMSG("PopState");
        this->deserialize(*state);
      }
    });

    mReturnStats.subscribe(MessageBus::GetGraphStats, [&](GraphStats** stats) {
      *stats = &mStats;
    });
  }

  void testadd() {
    return;
    Node* test = NodeList::createNode("SimpleCabNode");
    addNode(test, inputNode, 0);
    outputNode->connectInput(test->outSockets.Get(0));
  }

  ~Graph() {
    // TODO get rid of all the things
  }

  void OnReset(int p_sampleRate, int p_channles = 2) {
    if (p_sampleRate > 0 && p_channles > 0) {
      WDL_MutexLock lock(&isProcessing);
      sampleRate = p_sampleRate;
      channelCount = p_channles;
      inputNode->OnReset(p_sampleRate, p_channles);
      outputNode->OnReset(p_sampleRate, p_channles);
      for (int i = 0; i < nodes.GetSize(); i++) {
        nodes.Get(i)->OnReset(p_sampleRate, p_channles);
      }
    }
  }

  void ProcessBlock(iplug::sample** in, iplug::sample** out, int nFrames) {
    /**
     * I don't really like the mutex here, but it should only be locked if a change to the
     * processing chain is made, which will cause some artifacts anyways
     */
    auto start = std::chrono::high_resolution_clock::now();
    WDL_MutexLock lock(&isProcessing);
    if (nFrames > MAXBUFFER) {
      // TODO process this in smaller chunks, should be a simple for loop
      outputNode->CopyOut(out, nFrames);
      return;
    }
    inputNode->CopyIn(in, nFrames);

    int nodeCount = nodes.GetSize();
    for (int n = 0; n < nodeCount; n++) {
      nodes.Get(n)->BlockStart();
    }

    outputNode->BlockStart();

    // TODO multiple passes to ensure all the nodes are computed is super dumb
    int attempts = 0;
    while (!outputNode->isProcessed && attempts < 10) {
      for (int n = 0; n < nodeCount; n++) {
        nodes.Get(n)->ProcessBlock(nFrames);
      }
      outputNode->ProcessBlock(nFrames);
      attempts++;
    }

    outputNode->CopyOut(out, nFrames);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    mStats.executionTime = duration.count();
  }

  /** The graph needs to know about the graphics context to add and remove the controlls for the nodes */
  void setupUi(iplug::igraphics::IGraphics* pGraphics = nullptr) {
    if (pGraphics != nullptr && pGraphics != graphics) {
      WDBGMSG("Graphics context changed");
      graphics = pGraphics;
    }

    
    graphics->SetKeyHandlerFunc([&](const IKeyPress & key, bool isUp) {
      if (key.C && key.VK == kVK_Z && !isUp) {
        MessageBus::fireEvent<bool>(MessageBus::PopUndoState, false);
        return true;
      }
      return false;
    });

    background = new GraphBackground(graphics, [&](float x, float y, float scale) {
      this->onViewPortChange(x, y, scale);
    });
    graphics->AttachControl(background);

    for (int n = 0; n < nodes.GetSize(); n++) {
        nodes.Get(n)->setupUi(graphics);
    }
    inputNode->setupUi(graphics);
    outputNode->setupUi(graphics);

    cableLayer = new CableLayer(graphics, &nodes, outputNode);
    graphics->AttachControl(cableLayer);

    nodeGallery = new NodeGallery(graphics);
    graphics->AttachControl(nodeGallery);

    scaleUi();

    testadd();
  }

  void scaleUi() {
    if (width != 0 && height != 0 && scale != 0 && graphics != nullptr) {
      background->mScale = scale;
      graphics->Resize(width, height, scale);
    }
  }

  void cleanupUi() {
    width = graphics->Width();
    height = graphics->Height();
    scale = graphics->GetDrawScale();
    for (int n = 0; n < nodes.GetSize(); n++) {
      nodes.Get(n)->cleanupUi(graphics);
    }

    graphics->RemoveControl(nodeGallery, true);
    nodeGallery = nullptr;

    graphics->RemoveControl(background, true);
    background = nullptr;

    graphics->RemoveControl(cableLayer, true);
    cableLayer = nullptr;

    inputNode->cleanupUi(graphics);
    outputNode->cleanupUi(graphics);

    graphics = nullptr;
  }

  /**
   * Called via a callback from the background
   */
  void onViewPortChange(float dX = 0, float dY = 0, float scale = 1) {
    for (int i = 0; i < nodes.GetSize(); i++) {
      nodes.Get(i)->mUi->translate(dX, dY);
    }
    outputNode->mUi->translate(dX, dY);
    inputNode->mUi->translate(dX, dY);
    // WDBGMSG("x %f y %f s %f\n", x, y, scale);
  }

  void layoutUi(iplug::igraphics::IGraphics* pGraphics = nullptr) {
    if (pGraphics != nullptr && pGraphics != graphics) {
      WDBGMSG("Graphics context changed");
      graphics = pGraphics;
      // Todo find out whether the context ever changes
    }
    for (int n = 0; n < nodes.GetSize(); n++) {
      nodes.Get(n)->layoutChanged();
    }
  }

  /**
   * Used to add nodes and push a state to the history stack
   */
  void addNode(Node* node, Node* pInput = nullptr, int index = 0, float x = 0, float y = 0) {
    WDL_MutexLock lock(&isProcessing);
    node->X = x;
    node->Y = y;
    node->setup(sampleRate);
    paramManager.claimNode(node);
    node->setupUi(graphics);
    if (pInput != nullptr) {
      node->connectInput(pInput->outSockets.Get(index));
    }
    nodes.Add(node);
    sortRenderStack();
  }

  void removeAllNodes() {
    while (nodes.GetSize()) {
      removeNode(0);
    }
  }

  void removeNode(Node* node, bool reconnnect = false) {
    if (node == inputNode || node == outputNode) { return; }
    WDL_MutexLock lock(&isProcessing);
    if (reconnnect) {
      NodeSocket* prevSock = node->inSockets.Get(0);
      NodeSocket* nextSock = node->outSockets.Get(0);
      if (prevSock != nullptr && prevSock->connectedTo && nextSock != nullptr) {
        MessageBus::fireEvent<SocketConnectRequest>(
          MessageBus::SocketRedirectConnection,
          SocketConnectRequest {
            nextSock,
            prevSock->connectedTo
          }
        );
      }
    }
    node->cleanupUi(graphics);
    paramManager.releaseNode(node);
    nodes.DeletePtr(node, true);
    nodes.Compact();
  }

  void removeNode(int index) {
    removeNode(nodes.Get(index));
  }

  void serialize(nlohmann::json& json) {
    /** TODO See if this crashes on garageband/logic without a mutex */
    json["scale"] = scale;
    json["width"] = width;
    json["height"] = height;
    serializer::serialize(json, nodes, inputNode, outputNode);
  }

  void deserialize(nlohmann::json& json) {
    removeAllNodes();
    if (json.find("width") != json.end()) {
      scale = json["scale"];
      width = json["width"];
      height = json["height"];
    }
    WDL_MutexLock lock(&isProcessing);
    serializer::deserialize(json, nodes, outputNode, inputNode, sampleRate, &paramManager, graphics);
    sortRenderStack();
    scaleUi();
  }

private:
  /**
   * This just removes the overlay IControls and adds them again to
   * keep them on top of the layer stack
   */
  void sortRenderStack() {
    if (cableLayer != nullptr) {
      graphics->RemoveControl(cableLayer);
      graphics->AttachControl(cableLayer);
    }
    if (nodeGallery != nullptr) {
      graphics->RemoveControl(nodeGallery);
      graphics->AttachControl(nodeGallery);
    }
  }

};
