#pragma once
#include "src/graph/Node.h"
#include "StereoTool.h"


class StereoToolNode : public Node {
  StereoTool tool;
public:
  StereoToolNode() : Node() {
    type = "StereoToolNode";
  }

  void setup(ParameterManager* p_paramManager, int p_samplerate = 48000, int p_maxBuffer = 512, int p_inputs = 1, int p_outputs = 1, int p_channles = 2) {
    Node::setup(p_paramManager, p_samplerate, p_maxBuffer, 1, 1, 2);
    paramsFromFaust(&tool);
  }

  void ProcessBlock(int nFrames) {
    if (isProcessed) { return; }
    for (int i = 0; i < inputCount; i++) {
      if (!inputs[i]->isProcessed) {
        return;
      }
    }
    for (int i = 0; i < parameterCount; i++) {
      parameters[i]->update();
    }
    tool.compute(nFrames, inputs[0]->outputs[0], outputs[0]);
    isProcessed = true;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    background = new NodeBackground(pGrahics, PNGSTEREOSHAPERBG_FN, L, T,
      [&](float x, float y) {
        this->translate(x, y);
    });
    parameters[0]->x = 80;
    parameters[0]->y = 80;
    parameters[1]->x = 125;
    parameters[1]->y = 140;
    parameters[1]->w = 40;
    parameters[1]->h = 40;
    parameters[2]->x = 175;
    parameters[2]->y = 80;
    pGrahics->AttachControl(background);
    Node::setupUi(pGrahics);
  }
};

