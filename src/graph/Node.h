#pragma once

#include "IPlugConstants.h"
#include "src/logger.h"
#include "src/graph/ParameterManager.h"
#include <algorithm>
#include "IGraphics.h"
#include "src/faust/FaustHeadlessDsp.h"

class Node
{
  bool uiReady;
public:
  ParameterManager* paramManager;
  ParameterCoupling** parameters;
  int parameterCount;
  Node** inputs;
  iplug::sample*** outputs;

  int inputCount;
  int outputCount;
  bool isProcessed;

  Node(ParameterManager* p_paramManager, int p_samplerate, int p_maxBuffer = 512, int p_inputs = 1, int p_outputs = 1, int p_channles = 2) {
    paramManager = p_paramManager;
    samplerate = p_samplerate;
    maxBuffer = p_maxBuffer;
    inputCount = p_inputs;
    outputCount = p_outputs;
    isProcessed = false;
    channelCount = p_channles;
    parameterCount = 0;
    parameters = nullptr;
    uiReady = false;

    inputs = new Node*[std::max(1, p_inputs)];
    for (int i = 0; i < p_inputs; i++) {
      inputs[i] = nullptr;
    }

    outputs = new iplug::sample**[std::max(1, p_outputs)];
    for (int i = 0; i < p_outputs; i++) {
      outputs[i] = new iplug::sample*[p_channles];
      for (int c = 0; c < p_channles; c++) {
        outputs[i][c] = new iplug::sample[p_maxBuffer];
      }
    }
  };

  virtual ~Node() {
    delete inputs;
    for (int i = 0; i < outputCount; i++) {
      for (int c = 0; c < channelCount; c++) {
        delete outputs[i][c];
      }
      delete outputs[i];
    }
    delete outputs;

    if (uiReady) {
      WDBGMSG("Warning, UI of node was not cleaned up!");
    }

    // only delete the array, the UI struct in SimpleDelay will delete all the params inside
    for (int i = 0; i < parameterCount; i++) {
      // however the daw parameters still have to be freed so another node can take them if needed
      paramManager->releaseParameter(parameters[i]);
    }
    delete parameters;
  }

  virtual void ProcessBlock(int nFrames) = 0;

  virtual void ConnectInput(Node* in, int inputNumber = 0) {
    if (inputNumber < inputCount) {
      inputs[inputNumber] = in;
    }
  }

  virtual void DisconnectInput(int inputNumber = 0) {
    if (inputNumber < inputCount) {
      inputs[inputNumber] = nullptr;
    }
  }

  /**
   * Generic setup of the parameters to get something on the screen
   */
  virtual void setupUi(iplug::igraphics::IGraphics* pGrahics) {
    iplug::igraphics::IRECT bounds{ 0, 0, 600, 200 };
    int width = 80;
    for (int i = 0; i < parameterCount; i++) {
      ParameterCoupling* couple = parameters[i];
      iplug::igraphics::IRECT controlPos = bounds.GetFromLeft(width).GetHShifted(i * width);
      // use the daw parameter to sync the values if possible
      if (couple->parameterId != iplug::kNoParameter) {
        // All control objects attached to IGraphics will be deleted once the window
        // is destroyed, e.g the plugin windows was closed, so no need to clean these up
        couple->control = new iplug::igraphics::IVKnobControl(
          controlPos, couple->parameterId
        );
      }
      else {
        // use the callback to get tha value to the dsp, won't allow automation though
        couple->control = new iplug::igraphics::IVKnobControl(
          controlPos, [couple](iplug::igraphics::IControl* pCaller) {
            *(couple->value) = pCaller->GetValue();
          }
        );
      }
      // couple->control->SetValueToDefault();
      couple->control->SetValue(couple->defaultVal);
      pGrahics->AttachControl(couple->control);
    }
    uiReady = true;
  }

  virtual void cleanupUi(iplug::igraphics::IGraphics* pGrahics) {
    for (int i = 0; i < parameterCount; i++) {
      iplug::igraphics::IControl* control = parameters[i]->control;
      if (control != nullptr) {
        pGrahics->RemoveControl(control, true);
        control = nullptr;
      }
    }
    uiReady = false;
  }

  /**
   * This will use the UI shim to create ParameterCouplings between the faust dsp and iplug iControls
   */
  void paramsFromFaust(FaustHeadlessDsp* faust) {
    // This must be executed first since this will gather all the parameters from faust
    faust->setup(samplerate);

    // Keep the pointers around in a normal array since this might be faster than iterating over a vector
    parameters = new ParameterCoupling * [faust->ui.params.size()];
    parameterCount = 0;
    for (auto p : faust->ui.params) {
      parameters[parameterCount] = p;
      parameterCount++;
      if (!paramManager->claimParameter(p)) {
        // this means the manager has no free parameters left
        WDBGMSG("Ran out of daw parameters!");
      }
    }
  }

  virtual void layoutChanged() {

  }

  int samplerate;
  int channelCount;
  int maxBuffer;
};

