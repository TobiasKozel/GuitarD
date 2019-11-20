#include "GuitarD.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "src/nodes/RegisterNodes.h"


GuitarD::GuitarD(const InstanceInfo& info) : Plugin(info, MakeConfig(MAX_DAW_PARAMS, kNumPrograms)) {
  NodeList::registerNodes();
  graph = new Graph(&mBus);

  // Gather a good amount of parameters to expose to the daw based on what nodes are on the canvas
  for (int i = 0; i < MAX_DAW_PARAMS; i++) {
    graph->paramManager.addParameter(GetParam(i));
  }

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
    pGraphics->AttachCornerResizer(EUIResizerMode::Size, true);

    this->graph->setupUi(pGraphics);
  };
#endif
}

void GuitarD::OnReset() {
  if (graph != nullptr) {
    int sr = static_cast<int>(GetSampleRate());
    int ch = MaxNChannels(ERoute::kOutput);
    graph->OnReset(sr, ch);
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
  nlohmann::json serialized = {
    {"version", PLUG_VERSION_STR},
  };
  graph->serialize(serialized);
  chunk.PutStr(serialized.dump(4).c_str());
  return true;
}

int GuitarD::UnserializeState(const IByteChunk& chunk, int startPos) {
  WDL_String json_string;
  int pos = chunk.GetStr(json_string, startPos);
  try {
    nlohmann::json serialized = nlohmann::json::parse(json_string.Get());
    graph->deserialize(serialized);
    return pos;

  }
  catch (...) {
  }
  return pos;
  return IPluginBase::UnserializeParams(chunk, startPos);
}

#if IPLUG_DSP
void GuitarD::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
  graph->ProcessBlock(inputs, outputs, nFrames);
}
#endif
