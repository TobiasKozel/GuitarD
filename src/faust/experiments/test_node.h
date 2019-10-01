#pragma once

#include "src/node/Node.h"
#include "src/faust/experiments/test_faust.h"

class ReverbFaust : public Node {
public:
  double* gain;
  ReverbFaust(int p_samplerate, int p_maxBuffer = 512, int p_channels = 2) :
    Node(p_samplerate, p_maxBuffer, p_channels) {
    rev.setup(p_samplerate);
    gain = rev.getProperty("gain");
    if (!gain) {
      gain = new double;
    }
  }

  TestDsp rev;

  void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames) {
    rev.compute(nFrames, inputs, outputs);
  }

};