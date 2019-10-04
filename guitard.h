#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"
const int kNumPrograms = 1;
#include "src/graph/Graph.h"
#include "src/constants.h"
#include "src/graph/ParameterManager.h"

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
  Graph* graph;
  ParameterManager paramManager;
  IVButtonControl* testButton;
  IVKnobControl* cont;
#if IPLUG_DSP // All DSP methods and member variables should be within an IPLUG_DSP guard, should you want distributed UI
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif
};
