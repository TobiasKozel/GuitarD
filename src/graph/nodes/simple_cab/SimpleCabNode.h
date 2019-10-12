#pragma once
#include "src/graph/Node.h"
#include "config.h"
#include "src/graph/nodes/simple_cab/c.h"
#include "src/graph/nodes/simple_cab/c64.h"
#include "src/graph/nodes/simple_cab/cident.h"
#include "thirdparty/fftconvolver/TwoStageFFTConvolver.h"

class SimpleCabNode : public Node {
  fftconvolver::FFTConvolver convolver;
  float* convertBufferIn;
  float* convertBufferOut;
public:
  SimpleCabNode() : Node() {
    type = "SimpleCabNode";
    convolver.init(64, cIR, 512);
    convertBufferIn = new float[1024];
    convertBufferOut = new float[1024];
  }

  void setup(ParameterManager* p_paramManager, int p_samplerate = 48000, int p_maxBuffer = 512, int p_inputs = 1, int p_outputs = 1, int p_channles = 2) {
    Node::setup(p_paramManager, p_samplerate, p_maxBuffer, 1, 1, 2);
  }

  ~SimpleCabNode() {
    delete convertBufferIn;
    delete convertBufferOut;
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

    float inverseChannelCount = 1.0 / channelCount;
    for (int i = 0; i < nFrames; i++) {
      convertBufferIn[i] = 0;
      for (int c = 0; c < channelCount; c++) {
        convertBufferIn[i] += inputs[0]->outputs[0][c][i];
      }
      convertBufferIn[i] *= inverseChannelCount;
    }
    
    convolver.process(convertBufferIn, convertBufferOut, nFrames);
    
    for (int i = 0; i < nFrames; i++) {
      for (int c = 0; c < channelCount; c++) {
         outputs[0][c][i] = convertBufferOut[i];
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
