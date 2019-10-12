#pragma once
#include "src/graph/Node.h"
#include "config.h"
#include "src/graph/nodes/simple_cab/c.h"
#include "src/graph/nodes/simple_cab/c64.h"

class SimpleCabNode : public Node {
  iplug::sample** lastSignal;
public:
  SimpleCabNode() : Node() {
    type = "SimpleCabNode";
  }

  void setup(ParameterManager* p_paramManager, int p_samplerate = 48000, int p_maxBuffer = 512, int p_inputs = 1, int p_outputs = 1, int p_channles = 2) {
    lastSignal = new iplug::sample* [p_channles];
    outputs = new iplug::sample** [1];
    outputs[0] = new iplug::sample* [p_channles];

    for (int c = 0; c < p_channles; c++) {
      lastSignal[c] = new iplug::sample[1024];
      outputs[0][c] = (lastSignal[c] + 511);
    }
    Node::setup(p_paramManager, p_samplerate, p_maxBuffer, 1, 1, 2);
  }

  ~SimpleCabNode() {
    for (int c = 0; c < channelCount; c++) {
      delete lastSignal[c];
    }
    delete lastSignal;
    delete outputs[0];
    delete outputs;
    outputs = nullptr;
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
    for (int c = 0; c < channelCount; c++) {
      for (int i = 0; i < nFrames - 1; ++i) {
        outputs[0][c][i] = 0;                             // init to 0 before sum
        for (int j = i, k = 0; j >= 0; --j, ++k)
          outputs[0][c][i] += inputs[0]->outputs[0][c][j] * c64IR[k];
      }
    }
    isProcessed = true;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    background = new NodeBackground(pGrahics, PNGSIMPLECABBG_FN, L, T,
      [&](float x, float y) {
      this->translate(x, y);
    });
    pGrahics->AttachControl(background);
    uiReady = true;
    // Node::setupUi(pGrahics);
  }
};
