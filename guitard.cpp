#include "GuitarD.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "src/graph/nodes/simple_delay/SimpleDelayNode.h"
#include "src/graph/TestUiNode.h"

GuitarD::GuitarD(const InstanceInfo& info)
: Plugin(info, MakeConfig(MAXDAWPARAMS, kNumPrograms))
{

  // Gather a good amount of parameters to expose to the daw based on what nodes are on the canvas
  
  for (int i = 0; i < MAXDAWPARAMS; i++) {
    paramManager.addParameter(GetParam(i));
  }
  // TODO on mac garageband NOutChansConnected() reports zero for some reason
  graph = new Graph(&paramManager, GetSampleRate(), 2);

#if IPLUG_EDITOR // All UI methods and member variables should be within an IPLUG_EDITOR guard, should you want distributed UI
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    if (pGraphics->NControls()) {
      //Could handle new layout here
      return;
    }

    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, true);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    cont = nullptr;
    auto buttonAction = [&](IControl* pCaller) {
      //pCaller->SplashClickActionFunc();
      SplashClickActionFunc(pCaller);
      graph->testAdd();
      if (cont == nullptr) {
        cont = new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100), kGain);
        pGraphics->AttachControl(cont);
      }
      
    };
    pGraphics->AttachControl(
      new IVButtonControl(b.GetCentredInside(100).GetVShifted(100), buttonAction),
      kNoParameter, "vcontrols"
    );

    pGraphics->AttachControl(
      new UiNode(b.GetCentredInside(100).GetVShifted(200)),
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
