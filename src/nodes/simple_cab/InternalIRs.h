#pragma once

#include "clean.h"
#include "air.h"
#include "wdlstring.h"

struct IRBundle {
  WDL_String name;
  int channelCount = 1;
  int sampleRate = 48000;
  // Samplecount for a single channel
  size_t sampleCount = 0;
  float** samples = nullptr;
  // This is only set if it's a user IR
  WDL_String path;
};

IRBundle InternalIRs[] = {
  { WDL_String("Clean"), 1, 48000, InteralIR::cleanIRLength, InteralIR::cleanIR },
  { WDL_String("Air"), 1, 48000, InteralIR::airIRLength, InteralIR::airIR }
};

int InternalIRsCount = 2;