#include "GuitarD.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "src/graph/nodes/RegisterNodes.h"


GuitarD::GuitarD(const InstanceInfo& info) : Plugin(info, MakeConfig(MAXDAWPARAMS, kNumPrograms)) {
  NodeList::registerNodes();
  // TODO on mac garageband NOutChansConnected() reports zero for some reason
  graph = new Graph(static_cast<int>(GetSampleRate()), 2);

  // Gather a good amount of parameters to expose to the daw based on what nodes are on the canvas
  for (int i = 0; i < MAXDAWPARAMS; i++) {
    graph->paramManager.addParameter(GetParam(i));
  }

  mParamChanged.subscribe("ParametersChanged", [&](bool) {
    this->InformHostOfParameterDetailsChange();
  });


#if IPLUG_EDITOR // All UI methods and member variables should be within an IPLUG_EDITOR guard, should you want distributed UI
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    if (pGraphics->NControls()) {
      // If there are already controls in the context the layout function was only because of a resize
      this->graph->layoutUi(pGraphics);
      return;
    }
    pGraphics->SetSizeConstraints(400, 2000, 400, 1500);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    pGraphics->AttachCornerResizer(EUIResizerMode::Size, true);

    this->graph->setupUi(pGraphics);
  };
#endif
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
    {"ui_scale", 1.0} // TODO get the proper scale
  };
  graph->serialize(serialized);
  chunk.PutStr(serialized.dump(4).c_str());
  return true;
}

int GuitarD::UnserializeState(const IByteChunk& chunk, int startPos) {
  WDL_String json_string;
  chunk.GetStr(json_string, startPos);
  nlohmann::json serialized = nlohmann::json::parse(json_string.Get());
  graph->deserialize(serialized);
  return 0;
  return IPluginBase::UnserializeParams(chunk, startPos);
}

#if IPLUG_DSP
void GuitarD::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
  graph->ProcessBlock(inputs, outputs, nFrames);
}
#endif
