#pragma once
#include "../../node/Node.h"
#include "../../graph/Graph.h"

namespace guitard {

  class GraphNode final : public Node {
  public:
    Graph mGraph;
    GraphNode(NodeList::NodeInfo* info) {
      mInfo = info;
    }

    void setup(int pSamplerate, int pMaxBuffer, int, int, int) override {
      Node::setup(pSamplerate, pMaxBuffer, 1, 1, 2);
      addByPassParam();
    }

    void OnChannelsChanged(const int pChannels) override {
      Node::OnChannelsChanged(pChannels);
      mGraph.OnReset(mSampleRate, mChannelCount, mChannelCount);
    }

    void OnSamplerateChanged(const int pSampleRate) override {
      Node::OnSamplerateChanged(pSampleRate);
      mGraph.OnReset(mSampleRate, mChannelCount, mChannelCount);
    }

    void OnTransport() override {
      mGraph.OnTransport();
    }

    void ProcessBlock(int nFrames) override {
      if (byPass()) { return; }
      mGraph.ProcessBlock(mSocketsIn[0].mBuffer, mSocketsOut[0].mBuffer, nFrames);
    }

    void serializeAdditional(nlohmann::json& serialized) override {
      mGraph.serialize(serialized["state"]);
    }

    void deserializeAdditional(nlohmann::json& serialized) override {
      if (serialized.contains("state")) {
        mGraph.deserialize(serialized["state"]);
      }
    }
  };

  GUITARD_REGISTER_NODE(GraphNode, "Graph Meta Node", "Tools", "adasdsa")
}

#ifndef GUITARD_HEADLESS
#include "../../ui/NodeUi.h"
namespace guitard {
  class GraphNodeUi : public NodeUi {
  public:
    GraphNodeUi(Node* node, MessageBus::Bus* bus) : NodeUi(node, bus) { }

    void OnMouseDblClick(float x, float y, const IMouseMod& mod) override {
      MessageBus::fireEvent(mBus, MessageBus::EditMetaNode,
        &(static_cast<GraphNode*>(mNode)->mGraph)
      );
    }
  };

  GUITARD_REGISTER_NODE_UI(GraphNode, GraphNodeUi)
}
#endif
