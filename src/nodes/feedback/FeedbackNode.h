#pragma once
#include "src/node/Node.h"

class FeedbackNode final : public Node {
  bool hasLastBuffer;
  double gain;
  sample** prevBlock;
public:
  FeedbackNode(const std::string pType) : Node() {
    mType = pType;
  }

  void setup(MessageBus::Bus* pBus, int p_samplerate = 48000, int p_maxBuffer = MAX_BUFFER, int p_channels = 2, int p_inputs = 1, int p_outputs = 1) {
    Node::setup(pBus, p_samplerate, p_maxBuffer, p_channels, p_inputs, p_outputs);
    ParameterCoupling* p = new ParameterCoupling("Gain", &gain, 0.0, 0.0, 3.0, 0.01);
    shared.parameters[shared.parameterCount] = p;
    shared.parameterCount++;
    prevBlock = new sample * [p_channels];
    for (int c = 0; c < p_channels; c++) {
      prevBlock[c] = new sample[p_maxBuffer];
      for (int i = 0; i < p_maxBuffer; i++) {
        mBuffersOut[0][c][i] = 0;
        prevBlock[c][i] = 0;
      }
    }

    shared.socketsIn[0]->mX = shared.X + 100;
    shared.socketsOut[0]->mX = shared.Y - 100;
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
      shared.parameters[0]->update();
      sample** buffer = shared.socketsIn[0]->mConnectedTo->mParentBuffer;
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
    mUi->setColor(Theme::Categories::TOOLS);
  }
};