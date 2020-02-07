#pragma once

const int kNumPrograms = 2;
#include "soundwoofer/soundwoofer.h" // since winsocks does some weird stuff when being included after windows.h it needs to be included here though it's not used here
#include "IPlug_include_in_plug_hdr.h"

#include "src/graph/Graph.h"
#include "src/parameter/ParameterManager.h"

class GuitarD : public iplug::Plugin
{
  guitard::MessageBus::Subscription<bool> mParamChanged;
  // Each instance of a plugin has to have its own MessageBus
  guitard::MessageBus::Bus mBus;
  /** Is true when the plugin is ready to process samples (Knows the sample rate and in out channel count) */
  bool mReady = false;
  guitard::Graph* mGraph = nullptr;
  guitard::ParameterManager* mParamManager = nullptr;
public:
  GuitarD(const iplug::InstanceInfo& info);
  void OnUIClose() override;
  void OnReset() override;
  void OnActivate(bool active) override;

  bool SerializeState(iplug::IByteChunk& chunk) const override;
  int UnserializeState(const iplug::IByteChunk& chunk, int startPos) override;

/**
 * All DSP methods and member variables should be within an IPLUG_DSP guard, should you want distributed UI
 * However this isn't really an option for this plugin in the current state since a lot of the UI relies on
 * shared memory with the DSP
 */
#if IPLUG_DSP
  void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames) override;
#endif
};
