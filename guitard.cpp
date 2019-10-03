#include "GuitarD.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "src/graph/nodes/simple_delay/SimpleDelayNode.h"

GuitarD::GuitarD(const InstanceInfo& info)
: Plugin(info, MakeConfig(MAXDAWPARAMS, kNumPrograms))
{

  // Gather a good amount of parameters to expose to the daw based on what nodes are on the canvas
  
  for (int i = 0; i < MAXDAWPARAMS; i++) {
    paramManager.addParameter(GetParam(i));
  }

  graph = new Graph(&paramManager, GetSampleRate(), NOutChansConnected());

#if IPLUG_EDITOR // All UI methods and member variables should be within an IPLUG_EDITOR guard, should you want distributed UI
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100), kGain));


    auto buttonAction = [&](IControl* pCaller) {
      graph->testAdd();
    };
    pGraphics->AttachControl(
      new IVButtonControl(b.GetCentredInside(100).GetVShifted(100), buttonAction),
      kNoParameter, "vcontrols"
    );
  };
#endif
}

#if IPLUG_DSP
void GuitarD::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  graph->ProcessBlock(inputs, outputs, nFrames);
}
#endif
