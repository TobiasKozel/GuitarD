#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "src/faust/experiments/test_faust.h"

const int kNumPrograms = 1;

enum EParams
{
  kGain = 0,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class GuitarD : public Plugin
{
public:
  GuitarD(const InstanceInfo& info);
  TestDsp testsdp;
#if IPLUG_DSP // All DSP methods and member variables should be within an IPLUG_DSP guard, should you want distributed UI
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif
};
