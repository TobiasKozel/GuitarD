#pragma once
#include "src/node/Node.h"

class FeedbackNode : public Node {
  bool hasLastBuffer;
  double gain;
  sample** prevBlock;
public:
  FeedbackNode(std::string pType) : Node() {
    type = pType;
  }

  void setup(MessageBus::Bus* pBus, int p_samplerate = 48000, int p_maxBuffer = MAXBUFFER, int p_channels = 2, int p_inputs = 1, int p_outputs = 1) {
    Node::setup(pBus, p_samplerate, p_maxBuffer, p_channels, p_inputs, p_outputs);
    ParameterCoupling* p = new ParameterCoupling("Gain", &gain, 0.0, 0.0, 3.0, 0.01);
    parameters.Add(p);
    prevBlock = new sample * [p_channels];
    for (int c = 0; c < p_channels; c++) {
      prevBlock[c] = new sample[p_maxBuffer];
      for (int i = 0; i < p_maxBuffer; i++) {
        outputs[0][c][i] = 0;
        prevBlock[c][i] = 0;
      }
    }

    inSockets.Get(0)->X = X + 100;
    outSockets.Get(0)->X = X - 100;
    hasLastBuffer = false;
  }

  ~FeedbackNode() {
    for (int c = 0; c < channelCount; c++) {
      delete prevBlock[c];
    }
    delete prevBlock;
  }

  void BlockStart() override {
    isProcessed = false;
    hasLastBuffer = false;
  }

  void ProcessBlock(int nFrames) {
    isProcessed = true;
    if (inputsReady() && !hasLastBuffer) {
      parameters.Get(0)->update();
      sample** buffer = inSockets.Get(0)->connectedTo->parentBuffer;
      for (int c = 0; c < channelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          outputs[0][c][i] = prevBlock[c][i] * gain;
          prevBlock[c][i] = buffer[c][i];
        }
      }
      hasLastBuffer = true;
    }
  }
};