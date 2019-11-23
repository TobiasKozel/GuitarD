#pragma once
#include "ParametricEq.h"

class ParametricEqNodeUi final : public NodeUi {
public:
  ParametricEqNodeUi(NodeShared* param) : NodeUi(param) {
  }

  void setUpControls() override {
    NodeUi::setUpControls();
    for (int i = 0; i < shared->parameters.GetSize(); i++) {
      IControl* c = shared->parameters.Get(i)->control;
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
    float logW = w / 3.f;

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
};

class ParametricEqNode final : public ParametricEq {
public:
  ParametricEqNode(const std::string pType) {
    mType = pType;
    shared.width = 400;
    shared.height = 200;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    shared.graphics = pGrahics;
    mUi = new ParametricEqNodeUi(&shared);
    mUi->setColor(Theme::Categories::FILTER);
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    mUiReady = true;
  }
};