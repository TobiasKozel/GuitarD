#pragma once
#include "src/graph/Node.h"

class CombineNode : public Node {
  double pan1;
  double pan2;
  double mix;
  sample** emptyBuffer;
public:
  CombineNode() {
    pan1 = pan2 = 0;
    mix = 0.5;
    type = "StereoToolNode";
  }

  void ProcessBlock(int nFrames) {
    if (isProcessed) { return; }
    Node* node1 = inSockets.Get(0)->connectedNode;
    Node* node2 = inSockets.Get(1)->connectedNode;
    if (node1 == node2 && node1 == nullptr) {
      // Both inputs disconnected means nothing to do
      for (int c = 0; c < channelCount; c++) {
        for (int i = 0; i < nFrames; i++) {
          outputs[0][c][i] = 0;
        }
      }
      isProcessed = true;
      return;
    }

    if ((node1 != nullptr && !node1->isProcessed) || node2 != nullptr && !node2->isProcessed) {
      return;
    }

    sample** buffer1 = inSockets.Get(0)->buffer;
    sample** buffer2 = inSockets.Get(1)->buffer;
    buffer1 = buffer1 == nullptr ? emptyBuffer : buffer1;
    buffer2 = buffer2 == nullptr ? emptyBuffer : buffer2;
    parameters.Get(0)->update();
    parameters.Get(1)->update();
    parameters.Get(2)->update();
    double mix = *(parameters.Get(2)->value);
    double invMix = 1 - mix;
    for (int c = 0; c < channelCount; c++) {
      for (int i = 0; i < nFrames; i++) {
        outputs[0][c][i] = buffer1[c][i] * mix + buffer2[c][i] * invMix;
      }
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

    // this will be used to do processing on a disconnected node
    emptyBuffer = new sample* [channelCount];
    for (int c = 0; c < channelCount; c++) {
      emptyBuffer[c] = new sample[p_maxBuffer];
      for (int i = 0; i < p_maxBuffer; i++) {
        emptyBuffer[c][i] = 0;
      }
    }
  }
};