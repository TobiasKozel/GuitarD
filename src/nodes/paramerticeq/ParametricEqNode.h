#pragma once
#include "ParametricEq.h"

class ParametricEqNodeUi : public NodeUi {
public:
  ParametricEqNodeUi(NodeUiParam param) : NodeUi(param) {
  }

  void setUpControls() override {
    NodeUi::setUpControls();
    for (int i = 0; i < mParameters->GetSize(); i++) {
      IControl* c = mParameters->Get(i)->control;
      if (c != nullptr) {
        ////c->Hide(true);
      }
    }
  }

#define scaleLog(v, lw) log10(v * (100 / lw)) * lw - lw - 30
#define drawGuide(cr, cg, cb, v) g.DrawLine(IColor(255, cr, cg, cb), x + v, y - 100, x + v, y + 100)
  void DrawDebug(IGraphics& g, float x, float y, float w) {
    float logW = w / 3.f;
    float lowPassF = scaleLog(*(mParamsByName.at("lowF")->value), logW);
    drawGuide(255, 0, 0, lowPassF);
    float highPassF = scaleLog(*(mParamsByName.at("highF")->value), logW);
    drawGuide(0, 0, 255, highPassF);

    float f1 = scaleLog(*(mParamsByName.at("f1")->value), logW);
    drawGuide(0, 255, 0, f1);
    float f2 = scaleLog(*(mParamsByName.at("f2")->value), logW);
    drawGuide(255, 0, 255, f2);
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
      float offset = 0;
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

class ParametricEqNode : public ParametricEq {
public:
  ParametricEqNode(std::string pType) {
    type = pType;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    mUi = new ParametricEqNodeUi(NodeUiParam{
      mBus, pGrahics,
      300, 250,
      &X, &Y, &parameters, &inSockets, &outSockets, this
    });
    mUi->setColor(CATEGORYCOLORFILTER);
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    uiReady = true;
  }
};