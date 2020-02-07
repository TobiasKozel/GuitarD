#include "GuitarD.h"
#include "IPlug_include_in_plug_src.h"
#include "src/nodes/RegisterNodes.h"


GuitarD::GuitarD(const iplug::InstanceInfo& info) : iplug::Plugin(info, iplug::MakeConfig(MAX_DAW_PARAMS, kNumPrograms)) {
  guitard::NodeList::registerNodes();
  mParamManager = new guitard::ParameterManager(&mBus);

  for (int i = 0; i < MAX_DAW_PARAMS; i++) {
    // Gather a good amount of parameters to expose to the daw so they can be assigned internally
    mParamManager->addParameter(GetParam(i));
  }

  mGraph = new guitard::Graph(&mBus, mParamManager);

  /**
   * The DAW needs to be informed if the assignments for parameters have changed
   * TODOG This doesn't work for VST3 right now for some reason
   */
  mParamChanged.subscribe(&mBus, guitard::MessageBus::ParametersChanged, [&](bool) {
    this->InformHostOfParameterDetailsChange();
  });

  /**
   * Distributed UI won't work since the editor doesn't only afftect IParams
   * Syncing them up and seperatinh the gui from the main classes would require some work and 
   */
#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]() {
    return iplug::igraphics::MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };
  
  mLayoutFunc = [&](iplug::igraphics::IGraphics* pGraphics) {
    if (pGraphics->NControls()) {
      return;
    }
    pGraphics->SetSizeConstraints(PLUG_MIN_WIDTH, PLUG_MAX_WIDTH, PLUG_MIN_HEIGHT, PLUG_MAX_HEIGHT);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    pGraphics->LoadFont("ForkAwesome", ICON_FN);
    mGraph->setupUi(pGraphics);
  };
#endif
}

void GuitarD::OnReset() {
  if (mGraph != nullptr) {
    const int sr = static_cast<int>(GetSampleRate());
    const int outputChannels = NOutChansConnected();
    const int inputChannels = NInChansConnected();
    if (sr > 0 && outputChannels > 0 && inputChannels > 0) {
      // DAWs seem to
      mGraph->OnReset(sr, outputChannels, inputChannels);
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
  mGraph->cleanupUi();
}

bool GuitarD::SerializeState(iplug::IByteChunk& chunk) const {
  WDL_String serialized;
  mGraph->serialize(serialized);
  if (serialized.GetLength() < 1) {
    return false;
  }
  chunk.PutStr(serialized.Get());
  return true;
}

int GuitarD::UnserializeState(const iplug::IByteChunk& chunk, int startPos) {
  WDL_String json_string;
  const int pos = chunk.GetStr(json_string, startPos);
  mGraph->deserialize(json_string.Get());
  return pos;
}

#if IPLUG_DSP
void GuitarD::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames) {
  // Some DAWs already call the process function without having provided a valid samplerate/channel configuration
  if (mReady) {
    mGraph->ProcessBlock(inputs, outputs, nFrames);
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
