#pragma once

const int kNumPrograms = 2;
#include "json.hpp"
#include "soundwoofer/soundwoofer.h" // since winsocks does some weird stuff when being included after windows.h it needs to be included here though it's not used here
#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"
#include "src/misc/constants.h"
#include "src/graph/Graph.h"

using namespace iplug;
using namespace igraphics;

class GuitarD : public Plugin
{
  MessageBus::Subscription<bool> mParamChanged;
  // Each instance of a plugin has to have its own MessageBus
  MessageBus::Bus mBus;
  /** Is true when the plugin is ready to process samples (Knows the samplerate and in out channel count) */
  bool mReady = false;
public:
  GuitarD(const InstanceInfo& info);
  Graph* graph;
  void OnUIClose() override;
  void OnReset() override;
  void OnActivate(bool active) override;

  bool SerializeState(IByteChunk& chunk) const override;
  int UnserializeState(const IByteChunk& chunk, int startPos) override;

#if IPLUG_DSP // All DSP methods and member variables should be within an IPLUG_DSP guard, should you want distributed UI
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif
};
