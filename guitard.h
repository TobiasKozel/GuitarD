#pragma once

const int kNumPrograms = 2;
#define SOUNDWOOFER_IMPL
#include "./thirdparty/soundwoofer/soundwoofer.h" // since winsocks does some weird stuff when being included after windows.h it needs to be included here though it's not used here
#include "IPlug_include_in_plug_hdr.h"

#include "./src/main/Graph.h"
#include "./src/ui/elements/GraphUi.h"
#include "./src/main/parameter/ParameterManager.h"

class GuitarD : public iplug::Plugin
{
  // Each instance of a plugin has to have its own MessageBus
  guitard::MessageBus::Bus mBus;
  /** Is true when the plugin is ready to process samples (Knows the sample rate and in out channel count) */
  bool mReady = false;
  guitard::Graph* mGraph = nullptr;
  guitard::GraphUi* mGraphUi = nullptr;
  guitard::ParameterManager* mParamManager = nullptr;
public:
  GuitarD(const iplug::InstanceInfo& info);

  /**
   * Called when the window closes to free all the resources allocated by the GUI
   * The gui open function will be defined as a lambda inside the constructor of this class
   * by setting mMakeGraphicsFunc
   */
  void OnUIClose() override;

  /**
   * Called from the outside on a transport
   * At least I think it will?
   */
  void OnReset() override;

  /**
   * Called from outside when activated or deactivated
   * This doesn't work the same on all DAWs
   */
  void OnActivate(bool active) override;

  /**
   * Called from outside when a state needs to be saved
   */
  bool SerializeState(iplug::IByteChunk& chunk) const override;

  /**
   * Called from outside with a byte chunk to load
   */
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
