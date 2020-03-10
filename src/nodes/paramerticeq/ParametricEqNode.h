#pragma once
#include "../../faust/generated/ParametricEq.h"

namespace guitard {
  class ParametricEqNode final : public FaustGenerated::ParametricEq {
  public:
    ParametricEqNode(NodeList::NodeInfo* info) {
      mInfo = info;
      mDimensions.x = 400;
      mDimensions.y = 200;
    }

//#ifndef GUITARD_HEADLESS
//    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
//      shared.graphics = pGrahics;
//      mUi = new ParametricEqNodeUi(&shared);
//      mUi->setColor(Theme::Categories::FILTER);

//      pGrahics->AttachControl(mUi);
//      mUi->setUp();
//      mUiReady = true;
//    }
//#endif
  };

  GUITARD_REGISTER_NODE(ParametricEqNode,
    "Parametric Equalizer", "Filters",
    "Probably the worst eq in the history of eqs", "image"
  )

#ifndef GUITARD_HEADLESS
  class ParametricEqNodeUi final : public NodeUi {
  public:
    ParametricEqNodeUi(Node* node, MessageBus::Bus* bus) : NodeUi(node, bus) {
    }

    void setUpControls() override {
      NodeUi::setUpControls();
      for (int i = 0; i < mNode->mParameterCount; i++) {
        IControl* c = mNode->mParameters[i].control;
        if (c != nullptr) {
          ////c->Hide(true);
        }
      }
    }

#define SCALE_LOG(v, lw) log10(v * (100 / lw)) * lw - lw - 30
#define DRAW_GUIDE(cr, cg, cb, v) g.DrawLine(IColor(255, cr, cg, cb), x + v, y - 100, x + v, y + 100)
    void DrawDebug(IGraphics& g, float x, float y, float w) {
      const float logW = w / 3.f;
      const float lowPassF = SCALE_LOG(mParamsByName.at("lowF")->getWithAutomation<float>(), logW);
      DRAW_GUIDE(255, 0, 0, lowPassF);
      const float highPassF = SCALE_LOG(mParamsByName.at("highF")->getWithAutomation<float>(), logW);
      DRAW_GUIDE(0, 0, 255, highPassF);

      const float f1 = SCALE_LOG(mParamsByName.at("f1")->getWithAutomation<float>(), logW);
      DRAW_GUIDE(0, 255, 0, f1);
      const float f2 = SCALE_LOG(mParamsByName.at("f2")->getWithAutomation<float>(), logW);
      DRAW_GUIDE(255, 0, 255, f2);
    }

#define STEPSIZE 8
    void Draw(IGraphics& g) override {
      NodeUi::Draw(g);
      float x = mTargetRECT.L;
      int i = 0;
      float y = mTargetRECT.T + mTargetRECT.H() / 2.f;
      float w = mTargetRECT.W();
      DrawDebug(g, x, y, w);
      // float logW = w / 3.f;

      while (x < mTargetRECT.R) {
        const float offset = 0;
        g.DrawLine(IColor(255, 255, 0, 0), x, y + offset, x + STEPSIZE, y + offset);
        x += STEPSIZE;
        i++;
      }
      // DEBUG this only needs to be set dirty when the mouse moves
      mDirty = true;
    }

    void OnMouseOver(float x, float y, const IMouseMod& mod) override {
      IControl::OnMouseOver(x, y, mod);
    }

    void placeControls() override {
      const float size = 60;
      const float padX = 32;
      const float padY = 0;
      const float left = -mNode->mDimensions.x * 0.5 + 60;
      const float left2 = left + size + padX;
      const float left3 = left2 + size + padX;
      const float left4 = left3 + size + padX;
      const float top = -mNode->mDimensions.y * 0.5 + 50;
      const float top2 = top + padY + size;
      const float top3 = top2 + padY + size;
      mParamsByName.at("highF")->setPos(left, top, size);
      mParamsByName.at("highQ")->setPos(left, top2, size);

      mParamsByName.at("f1")->setPos(left2, top, size);
      mParamsByName.at("peak1")->setPos(left2, top2, size);
      mParamsByName.at("q1")->setPos(left2, top3, size);

      mParamsByName.at("f2")->setPos(left3, top, size);
      mParamsByName.at("peak2")->setPos(left3, top2, size);
      mParamsByName.at("q2")->setPos(left3, top3, size);

      mParamsByName.at("lowF")->setPos(left4, top, size);
      mParamsByName.at("lowQ")->setPos(left4, top2, size);
    }
  };

  GUITARD_REGISTER_NODE_UI(ParametricEqNode, ParametricEqNodeUi)
#endif
}
