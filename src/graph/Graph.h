#pragma once
#include "mutex.h"
#include "thirdparty/json.hpp"
#include "IPlugConstants.h"
#include "src/constants.h"
#include "src/logger.h"
#include "src/graph/Node.h"
#include "src/graph/nodes/DummyNode.h"
#include "src/graph/ui/GraphBackground.h"
#include "src/graph/misc/Serializer.h"
#include "src/graph/misc/ParameterManager.h"
#include "src/graph/ui/NodeSocketUi.h"
#include "src/graph/ui/CableLayer.h"
#include "src/graph/ui/NodeGallery.h"

class Graph {
  iplug::igraphics::IGraphics* graphics;
  /** Holds all the nodes in the processing graph */
  WDL_PtrList<Node> nodes;
  WDL_Mutex isProcessing;
  /** Dummy nodes to get the audio blocks in and out of the graph */
  DummyNode* input;
  DummyNode* output;
  int channelCount;

  GraphBackground* background;

  CableLayer* cableLayer;

  NodeGallery* nodeGallery;

public:
  ParameterManager paramManager;


  Graph(int p_sampleRate, int p_channles = 2) {
    graphics = nullptr;
    background = nullptr;
    nodeGallery = nullptr;
    sampleRate = p_sampleRate;
    channelCount = p_channles;
    input = new DummyNode(true, channelCount);
    output = new DummyNode(false, channelCount);
    NodeSocket* test = input->outSockets.Get(0);
    output->connectInput(test);
  }

  ~Graph() {
    // TODO get rid of all the things
    // not a priority since there is no use case for multiple/dynamic graphs
  }

  void ProcessBlock(iplug::sample** in, iplug::sample** out, int nFrames) {
    /**
     * I don't really like the mutex here, but it should only be locked if a change to the
     * processing chain is made, which will cause some artifacts anyways
     */
    WDL_MutexLock lock(&isProcessing);
    input->CopyIn(in, nFrames);

    int nodeCount = nodes.GetSize();
    for (int n = 0; n < nodeCount; n++) {
      nodes.Get(n)->isProcessed = false;
    }

    if (output->inSockets.Get(0)->buffer == nullptr || nFrames > MAXBUFFER) {
      // no out connected or too large of a block requested, so output nothing
      for (int c = 0; c < channelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          out[c][i] = 0;
        }
      }
      return;
    }
    // TODO multiple passes to ensure all the nodes are computed is super dumb
    while (!output->inSockets.Get(0)->connectedNode->isProcessed) {
      for (int n = 0; n < nodeCount; n++) {
        nodes.Get(n)->ProcessBlock(nFrames);
      }
    }

    output->CopyOut(out, nFrames);
  }

  void testAdd() {
    WDL_MutexLock lock(&isProcessing);
    if (nodes.GetSize() == 0) {
      Node* stereo = NodeList::createNode("StereoToolNode");
      addNode(stereo, input, 0, 0);
      //Node* delay = new SimpleDelayNode();
      //addNode(delay, stereo, 200);
      Node* drive = NodeList::createNode("SimpleDriveNode");
      addNode(drive, nullptr, 0, 400);
      //Node* baby = new CryBabyNode();
      //addNode(baby, drive, 600);
      Node* cab = NodeList::createNode("SimpleCabNode");
      addNode(cab, drive, 0, 800);
      output->connectInput(cab->outSockets.Get(0));
    }
    else {
      removeAllNodes();
      output->connectInput(input->outSockets.Get(0));
    }
  }

  /** The graph needs to know about the graphics context to add and remove the controlls for the nodes */
  void setupUi(iplug::igraphics::IGraphics* pGraphics = nullptr) {
    background = new GraphBackground(pGraphics, [&](float x, float y, float scale) {
        this->onViewPortChange(x, y, scale);
      },
      [&](float x, float y, const IMouseMod& mod) {
        if (mod.R) {
          this->toggleGallery();
        }
        else {
          this->toggleGallery(true);
        }
      }
    );
    pGraphics->AttachControl(background);

    if (pGraphics != nullptr && pGraphics != graphics) {
      WDBGMSG("Graphics context changed");
      graphics = pGraphics;
    }
    for (int n = 0; n < nodes.GetSize(); n++) {
        nodes.Get(n)->setupUi(pGraphics);
    }
    input->setupUi(pGraphics);
    output->setupUi(pGraphics);
    cableLayer = new CableLayer(pGraphics, &nodes, output);
    pGraphics->AttachControl(cableLayer);
  }

  void toggleGallery(bool wantsClose = false) {
    if (nodeGallery == nullptr && !wantsClose) {
      nodeGallery = new NodeGallery(graphics, [&](NodeList::NodeInfo info) {
        this->addNode(info.constructor(), nullptr, 0, 300, 300);
      });
      graphics->AttachControl(nodeGallery);
    }
    else if (nodeGallery != nullptr){
      graphics->RemoveControl(nodeGallery, true);
      nodeGallery = nullptr;
    }
  }

  void cleanupUi() {
    for (int n = 0; n < nodes.GetSize(); n++) {
      nodes.Get(n)->cleanupUi(graphics);
    }
    graphics->RemoveControl(background, true);
    graphics->RemoveControl(cableLayer, true);
    input->cleanupUi(graphics);
    output->cleanupUi(graphics);
    cableLayer = nullptr;
    graphics = nullptr;
  }

  /**
   * Called via a callback from the background
   */
  void onViewPortChange(float x, float y, float scale) {
    for (int i = 0; i < nodes.GetSize(); i++) {
      nodes.Get(i)->mUi->translate(x, y);
    }
    output->mUi->translate(x, y);
    input->mUi->translate(x, y);
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

  void deserialize(nlohmann::json& json) {
    WDL_MutexLock lock(&isProcessing);
    removeAllNodes();
    serializer::deserialize(json, nodes, output, input, sampleRate, &paramManager, graphics);
    sortRenderStack();
  }

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

  void removeNode(int index) {
    WDL_MutexLock lock(&isProcessing);
    Node* node = nodes.Get(index);
    if (node != nullptr) {
      if (node == output->inSockets.Get(0)->connectedNode) {
        output->disconnectInput(0);
      }
      node->cleanupUi(graphics);
      paramManager.releaseNode(node);
      nodes.DeletePtr(node, true);
    }
  }

  void serialize(nlohmann::json& json) {
    /**
     * TODO this shouldn't need a lock since we don't want stutter when autosaves etc
     * are in progress. Without it crashes about 50% of the time
     */
    WDL_MutexLock lock(&isProcessing);
    json["output"] = {
      nodes.Find(output->inSockets.Get(0)->connectedNode),
      output->inSockets.Get(0)->connectedBufferIndex
    };
    serializer::serialize(json, nodes, input);
  }

private:

  void sortRenderStack() {
    // keep the cable layer on top
    graphics->RemoveControl(cableLayer);
    graphics->AttachControl(cableLayer);
    if (nodeGallery != nullptr) {
      graphics->RemoveControl(nodeGallery);
      graphics->AttachControl(nodeGallery);
    }
  }

  int sampleRate;
};
