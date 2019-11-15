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

#define STEPSIZE 8
  void Draw(IGraphics& g) override {
    NodeUi::Draw(g);
    float x = mRECT.L;
    int i = 0;
    float y = mRECT.T + mRECT.H() / 2.f;
    float width = mRECT.W();
    float logW = width / 3.f;
    // g.DrawLine(IColor(255, 255, 0, 0), x, y - 10 * *(mParamsByName.at("lowF")->value), x + 60, y);
    float lowPassF = log10(*(mParamsByName.at("lowF")->value) * (100 / logW)) * logW - logW - 30;
    g.DrawLine(IColor(255, 255, 0, 0), x + lowPassF, y - 100, x + lowPassF, y + 100);
    //while (x < mRECT.R) {
    //  g.DrawLine(IColor(255, 255, 0, 0), x, y - 100, x, y + 100);
    //  x += STEPSIZE;
    //  i++;
    //}

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
      pGrahics,
      IColor(255, 100, 100, 150),
      300, 250,
      &X, &Y,
      &parameters,
      &inSockets,
      &outSockets,
      this
      });
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    uiReady = true;
  }
};