#pragma once

#include "clean.h"
#include "air.h"

struct IRBundle {
  const char* name = nullptr;
  int channelCount = 1;
  int sampleRate = 48000;
  // Samplecount for a single channel
  int sampleCount = 0;
  float** samples = nullptr;
  // This is only set if it's a user IR
  const char* path = nullptr;
};

IRBundle InternalIRs[] = {
  { "Clean", 1, 48000, InteralIR::cleanIRLength, InteralIR::cleanIR },
  { "Air", 1, 48000, InteralIR::airIRLength, InteralIR::airIR }
};
