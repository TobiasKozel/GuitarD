#pragma once
#include "src/node/Node.h"

class FeedbackNode : public Node {
  bool hasLastBuffer;
  double gain;
  sample** prevBlock;
public:
  FeedbackNode(std::string pType) : Node() {
    mType = pType;
  }

  void setup(MessageBus::Bus* pBus, int p_samplerate = 48000, int p_maxBuffer = MAX_BUFFER, int p_channels = 2, int p_inputs = 1, int p_outputs = 1) {
    Node::setup(pBus, p_samplerate, p_maxBuffer, p_channels, p_inputs, p_outputs);
    ParameterCoupling* p = new ParameterCoupling("Gain", &gain, 0.0, 0.0, 3.0, 0.01);
    mParameters.Add(p);
    prevBlock = new sample * [p_channels];
    for (int c = 0; c < p_channels; c++) {
      prevBlock[c] = new sample[p_maxBuffer];
      for (int i = 0; i < p_maxBuffer; i++) {
        mBuffersOut[0][c][i] = 0;
        prevBlock[c][i] = 0;
      }
    }

    mSocketsIn.Get(0)->X = mX + 100;
    mSocketsOut.Get(0)->X = mX - 100;
    hasLastBuffer = false;
  }

  ~FeedbackNode() {
    for (int c = 0; c < mChannelCount; c++) {
      delete prevBlock[c];
    }
    delete prevBlock;
  }

  void BlockStart() override {
    mIsProcessed = false;
    hasLastBuffer = false;
  }

  void ProcessBlock(int nFrames) {
    mIsProcessed = true;
    if (inputsReady() && !hasLastBuffer) {
      mParameters.Get(0)->update();
      sample** buffer = mSocketsIn.Get(0)->connectedTo->parentBuffer;
      for (int c = 0; c < mChannelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          mBuffersOut[0][c][i] = prevBlock[c][i] * gain;
          prevBlock[c][i] = buffer[c][i];
        }
      }
      hasLastBuffer = true;
    }
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    Node::setupUi(pGrahics);
    mUi->setColor(CATEGORYCOLORTOOLS);
  }
};