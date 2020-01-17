#include "GuitarD.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "src/nodes/RegisterNodes.h"
#include "src/misc/DefaultPreset.h"


GuitarD::GuitarD(const InstanceInfo& info) : Plugin(info, MakeConfig(MAX_DAW_PARAMS, kNumPrograms)) {
  
  NodeList::registerNodes();
  graph = new Graph(&mBus);
  // Gather a good amount of parameters to expose to the daw based on what nodes are on the canvas
  for (int i = 0; i < MAX_DAW_PARAMS; i++) {
    graph->mParamManager.addParameter(GetParam(i));
  }

  IByteChunk factoryPreset;
  factoryPreset.PutBytes(DEFAULT_PRESET_STRING, strlen(DEFAULT_PRESET_STRING));
  MakePresetFromChunk("Factory Preset", factoryPreset);

  mParamChanged.subscribe(&mBus, MessageBus::ParametersChanged, [&](bool) {
    this->InformHostOfParameterDetailsChange();
  });

  /**
   * Distributed UI won't work since the editor doesn't only afftect IParams
   * Syncing them up and seperatinh the gui from the main classes would require some work and 
   */
#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    if (pGraphics->NControls()) {
      // If there are already controls in the context the layout function was only because of a resize
      this->graph->layoutUi(pGraphics);
      return;
    }
    pGraphics->SetSizeConstraints(PLUG_MIN_WIDTH, PLUG_MAX_WIDTH, PLUG_MIN_HEIGHT, PLUG_MAX_HEIGHT);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    pGraphics->LoadFont("ForkAwesome", ICON_FN);
    this->graph->setupUi(pGraphics);
  };
#endif
}

void GuitarD::OnReset() {
  if (graph != nullptr) {
    const int sr = static_cast<int>(GetSampleRate());
    const int outputChannels = NOutChansConnected();
    const int inputChannels = NInChansConnected();
    if (sr > 0 && outputChannels > 0 && inputChannels > 0) {
      // DAWs seem to
      graph->OnReset(sr, outputChannels, inputChannels);
      mReady = true;
    }
  }
}

void GuitarD::OnActivate(bool active) {
  if (!active && mReady) {
    OnReset();
  }
}

void GuitarD::OnUIClose() {
  /**
   * The gui will be cleaned up when the Iplug window is destructed
   * however doing this manually will be safer and make sure all the nodes can clean up
   * after them selves and set the control in the ParameterCoupling to a nullptr
   */
  graph->cleanupUi();
}

bool GuitarD::SerializeState(IByteChunk& chunk) const {
  WDL_String serialized;
  graph->serialize(serialized);
  if (serialized.GetLength() < 1) {
    return false;
  }
  chunk.PutStr(serialized.Get());
  return true;
}

int GuitarD::UnserializeState(const IByteChunk& chunk, int startPos) {
  WDL_String json_string;
  const int pos = chunk.GetStr(json_string, startPos);
  graph->deserialize(json_string.Get());
  return pos;
}

#if IPLUG_DSP
void GuitarD::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
  // Some DAWs already call the process function without having provided a valid samplerate/channel configuration
  if (mReady) {
    graph->ProcessBlock(inputs, outputs, nFrames);
  }
  else {
    OnReset();
    // Fill the output buffer just in case it wasn't initialized with silence
    const int nChans = NOutChansConnected();
    for (int c = 0; c < nChans; c++) {
      for (int i = 0; i < nFrames; i++) {
        outputs[c][i] = inputs[c][i];
      }
    }
  }
}
#endif
