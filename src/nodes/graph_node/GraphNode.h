#pragma once
#include "../../main/Node.h"
#include "../../main/Graph.h"

namespace guitard {

  class GraphNode final : public Node {
  public:
    Graph mGraph;

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

  GUITARD_REGISTER_NODE(GraphNode, "Graph Meta Node", "Tools", "A meta node which can hold its own graph inside")
}

#ifndef GUITARD_HEADLESS
#include "../../ui/elements/NodeUi.h"
namespace guitard {
  class GraphNodeUi : public NodeUi {
  public:
    GraphNodeUi(Node* node, MessageBus::Bus* bus) : NodeUi(node, bus) { }
    void Draw(IGraphics& g) override {
      NodeUi::Draw(g);
      IRECT padded = mRECT.GetPadded(-40);
      g.DrawText(Theme::Gallery::ELEMENT_TITLE, "Double click to enter node", padded.GetReducedFromBottom(20));
      g.DrawText(Theme::Gallery::ELEMENT_TITLE, "Double click background to leave", padded);
      
    }

    void OnMouseDblClick(float x, float y, const IMouseMod& mod) override {
      MessageBus::fireEvent(mBus, MessageBus::EditMetaNode,
        &(static_cast<GraphNode*>(mNode)->mGraph)
      );
    }
  };

  GUITARD_REGISTER_NODE_UI(GraphNode, GraphNodeUi)
}
#endif
