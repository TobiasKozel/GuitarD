#pragma once

#include "IPlugConstants.h"

class Node
{
public:

  Node(int p_samplerate, int p_maxBuffer = 512, int p_channels = 2) {
    samplerate = p_samplerate;
    maxBuffer = p_maxBuffer;
    channels = p_channels;

  };

  ~Node() {
  }

  virtual void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames) = 0;

private:
  int samplerate;
  int maxBuffer;
  int channels;
};

