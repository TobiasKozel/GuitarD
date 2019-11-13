#pragma once
#include "src/node/Node.h"

class CombineNode : public Node {
  double pan1;
  double pan2;
  double mix;
  sample** emptyBuffer;
public:
  CombineNode(std::string pType) : Node() {
    pan1 = pan2 = 0;
    mix = 0.5;
    type = pType;
  }

  void ProcessBlock(int nFrames) {
    if (isProcessed) { return; }
    NodeSocket* s1 = inSockets.Get(0);
    NodeSocket* s2 = inSockets.Get(1);

    // see which inputs are connected
    bool has1 = s1->connectedNode != nullptr;
    bool has2 = s2->connectedNode != nullptr;
    if (has1 == has2 && has1 == false) {
      outputSilence();
      return;
    }

    if ((has1 && !s1->connectedNode->isProcessed) || has2 && !s2->connectedNode->isProcessed) {
      // skip until inputs are ready
      return;
    }

    // Choose the buffer from the input or use silence
    sample** buffer1 = has1 ? s1->connectedTo->parentBuffer : emptyBuffer;
    sample** buffer2 = has2 ? s2->connectedTo->parentBuffer : emptyBuffer;

    // Update the params
    parameters.Get(0)->update();
    parameters.Get(1)->update();
    parameters.Get(2)->update();

    // prepare the values
    double mix = *(parameters.Get(2)->value);
    double invMix = 1 - mix;
    double pan1 = *(parameters.Get(0)->value);
    double pan2 = *(parameters.Get(1)->value);
    double pan1l = min(1.0, max(-pan1 + 1.0, 0.0)) * invMix;
    double pan1r = min(1.0, max(+pan1 + 1.0, 0.0)) * invMix;
    double pan2l = min(1.0, max(-pan2 + 1.0, 0.0)) * mix;
    double pan2r = min(1.0, max(+pan2 + 1.0, 0.0)) * mix;

    // do the math
    for (int i = 0; i < nFrames; i++) {
      outputs[0][0][i] = buffer1[0][i] * pan1l + buffer2[0][i] * pan2l;
    }
    for (int i = 0; i < nFrames; i++) {
      outputs[0][1][i] = buffer1[1][i] * pan1r + buffer2[1][i] * pan2r;
    }

    isProcessed = true;
  }

  void setup(int p_samplerate = 48000, int p_maxBuffer = 512, int p_channles = 2, int p_inputs = 1, int p_outputs = 1) {
    Node::setup(p_samplerate, p_maxBuffer, 2, 2, 1);
    ParameterCoupling* p = new ParameterCoupling(
      "PAN 1", &pan1, 0.0, -1.0, 1.0, 0.01
    );
    p->x = -100;
    p->y = -100;
    parameters.Add(p);

    p = new ParameterCoupling(
      "PAN 2", &pan2, 0.0, -1.0, 1.0, 0.01
    );
    p->x = -100;
    p->y = 100;
    parameters.Add(p);

    p = new ParameterCoupling(
      "MIX", &mix, 0.5, 0.0, 1.0, 0.01
    );
    p->x = 0;
    p->y = 0;
    parameters.Add(p);
  }

  void deleteBuffers() override {
    Node::deleteBuffers();
    if (emptyBuffer != nullptr) {
      for (int c = 0; c < channelCount; c++) {
        delete emptyBuffer[c];
      }
      emptyBuffer = nullptr;
    }
  }

  void createBuffers() override {
    Node::createBuffers();
    // this will be used to do processing on a disconnected node
    emptyBuffer = new sample * [channelCount];
    for (int c = 0; c < channelCount; c++) {
      emptyBuffer[c] = new sample[maxBuffer];
      for (int i = 0; i < maxBuffer; i++) {
        emptyBuffer[c][i] = 0;
      }
    }
  }
};