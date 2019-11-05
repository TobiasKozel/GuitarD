#pragma once
#include "thirdparty/fftconvolver/TwoStageFFTConvolver.h"
#include "resample.h"
#include "config.h"
#include "src/node/Node.h"
#include "src/nodes/simple_cab/c.h"
#include "src/nodes/simple_cab/cident.h"
#include "src/nodes/simple_cab/clean.h"

class SimpleCabNode : public Node {
  fftconvolver::FFTConvolver convolver;
  fftconvolver::FFTConvolver convolver2;
  WDL_Resampler mResampler;
  double* resampledIR;
  double selectedIr;
public:
  SimpleCabNode(std::string pType) : Node() {
    type = pType;
    resampledIR = nullptr;
    selectedIr = 0;
  }

  // TODO FIX THIS MESS
  void setup(int p_samplerate = 48000, int p_maxBuffer = 512, int p_channles = 2, int p_inputs = 1, int p_outputs = 1) {
    Node::setup(p_samplerate, p_maxBuffer, 2, 1, 1);
    //ParameterCoupling* p = new ParameterCoupling("IR", &selectedIr, 0.0, 0.0, 2.0, 1.0);
    //parameters.Add(p);
    mResampler.SetMode(true, 0, true);
    mResampler.SetFilterParms();
    mResampler.SetFeedMode(true);
    mResampler.SetRates(48000, p_samplerate);
    double* test;
    int inSamples = mResampler.ResamplePrepare(3000, 1, &test);
    for (int i = 0; i < 3000; i++) {
      test[i] = cleanIR[i];
    }
    resampledIR = new double[10000];
    int outSamples = mResampler.ResampleOut(resampledIR, inSamples, 8000, 1);
    convolver.init(64, resampledIR, outSamples);
    convolver2.init(64, resampledIR, outSamples);
    //convolver.init(64, cleanIR, 3000);
    mStereo = 0;
    addByPassParam();
    addStereoParam();
  }

  ~SimpleCabNode() {
    delete resampledIR;
  }

  void ProcessBlock(int nFrames) {
    if (!inputsReady() || isProcessed || byPass()) { return; }
    parameters.Get(1)->update();
    //int prev = (int)*(parameters[0]->value);
    //parameters[0]->update();
    //int cur = (int)*(parameters[0]->value);
    //if (prev != cur) {
    //  if (cur == 0) {
    //    convolver.init(64, cleanIR, 3000);
    //  }
    //  if (cur == 1) {
    //    convolver.init(64, stackIR, 4500);
    //  }
    //  if (cur == 2) {
    //    convolver.init(64, driveIR, 4200);
    //  }
    //}

    sample** buffer = inSockets.Get(0)->connectedTo->parentBuffer;


    convolver.process(buffer[0], outputs[0][0], nFrames);
    if (mStereo) {
      convolver2.process(buffer[1], outputs[0][1], nFrames);
    }
    else {
      for (int i = 0; i < nFrames; i++) {
        outputs[0][1][i] = outputs[0][0][i];
      }
    }
    

    isProcessed = true;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    //background = new NodeBackground(pGrahics, PNGSIMPLECABBG_FN, L, T,
    //  [&](float x, float y) {
    //  this->translate(x, y);
    //});
    //pGrahics->AttachControl(background);
    Node::setupUi(pGrahics);
    // uiReady = true;
  }
};
