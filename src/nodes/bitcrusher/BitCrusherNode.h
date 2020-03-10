#pragma once
#include "../../faust/generated/BitCrusher.h"

namespace guitard {
  class BitCrusherNode final : public FaustGenerated::BitCrusher {
  public:
    explicit BitCrusherNode(NodeList::NodeInfo* info) {
      mInfo = info;
      mDimensions.x = 250;
      mDimensions.y = 240;
    }
  };

  GUITARD_REGISTER_NODE(BitCrusherNode,
    "Bitcrusher", "Distortion", "Bit crush effect for Lo-Fi 8 Bit sound", "image"
  )

#ifndef GUITARD_HEADLESS
  class BitCrusherNodeUi : public NodeUi {
    public:
      BitCrusherNodeUi(Node* node, MessageBus::Bus* bus) : NodeUi(node, bus) {
        setSvg(SVGBITTERBG_FN);
        const float w = mNode->mDimensions.x;
        const float h = mNode->mDimensions.y;
        const float left1 = w * 0.26 - w / 2;
        const float left2 = w * 0.68 - w / 2;
        const float top1 =  w * 0.35 - h / 2;
        const float top2 =  w * 0.7 -  h / 2;
        const float size = 60;
        mParamsByName.at("Bits")->setPos(left1, top1, size, false);
        mParamsByName.at("Downsampling Factor")->setPos(left2, top1, size, false);
        mParamsByName.at("Mix")->setPos(left1, top2, size, false);
      }
  };

  GUITARD_REGISTER_NODE_UI(BitCrusherNode, BitCrusherNodeUi)
#endif
}