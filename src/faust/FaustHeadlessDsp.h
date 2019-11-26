#pragma once

#define FAUSTFLOAT iplug::sample

#include "IPlugConstants.h"
#include "src/misc/constants.h"
#include "src/parameter/ParameterCoupling.h"
#include "src/node/Node.h"


struct Meta {
  virtual void declare(const char* key, const char* value) = 0;
};

/** 
 * This is a shim to collect pointers to all the properties/parameters from the faust DSP code
 */
struct UI {
  WDL_PtrList<ParameterCoupling>* params;
  WDL_PtrList<MeterCoupling>* meters;
  const char* name;

  UI(WDL_PtrList<ParameterCoupling>* pParams, WDL_PtrList<MeterCoupling>* pMeters) {
    params = pParams;
    meters = pMeters;
    name = DEFAULT_NODE_NAME;
  }

  void openVerticalBox(const char* key) {
    // NOTE This only is the name of the module if it has one box!
    if (name != DEFAULT_NODE_NAME) {
      WDBGMSG("openVerticalBox called multiple times. The node Type might be wrong!");
      assert(false);
    }
    name = key;
  };
  static void openHorizontalBox(const char* key) {};
  static void closeBox() {};
  static void declare(FAUSTFLOAT*, const char*, const char*) {};

  void addHorizontalSlider(const char* name, FAUSTFLOAT* prop, FAUSTFLOAT pDefault, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT stepSize) const {
    params->Add(new ParameterCoupling(name, prop, pDefault, min, max, stepSize));
  }

  void addVerticalSlider(const char* name, FAUSTFLOAT* prop, FAUSTFLOAT pDefault, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT stepSize) const {
    params->Add(new ParameterCoupling(name, prop, pDefault, min, max, stepSize));
  }

  void addCheckButton(const char* name, FAUSTFLOAT* prop) const {
    params->Add(new ParameterCoupling(name, prop, 0, 0, 1, 1));
  }

  void addVerticalBargraph(const char* name, FAUSTFLOAT* prop, FAUSTFLOAT min, FAUSTFLOAT max) const {
    meters->Add(new MeterCoupling{ prop, name, min, max });
  };

  void addHorizontalBargraph(const char* name, FAUSTFLOAT* prop, FAUSTFLOAT min, FAUSTFLOAT max) const {
    meters->Add(new MeterCoupling{ prop, name, min, max });
  };
};


/**
 * The faust DSP code will derive from this
 */
class FaustHeadlessDsp: public Node {
public:
  // These three will be overridden by the generated faust code
  virtual void init(int samplingFreq) = 0;
  virtual void buildUserInterface(UI* ui_interface) = 0;
  virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) = 0;
  virtual int getNumInputs() = 0;
  virtual int getNumOutputs() = 0;
  virtual void instanceConstants(int samplingFreq) = 0;
  virtual void instanceClear() = 0;

  void setup(MessageBus::Bus* pBus, const int pSamplerate = 48000, const int pMaxBuffer = MAX_BUFFER, const int pChannels = 2, int pInputs = 1, int pOutputs = 1) override {
    Node::setup(pBus, pSamplerate, pMaxBuffer, pChannels, getNumInputs() / pChannels, getNumOutputs() / pChannels);
    
    addByPassParam();
    /**
     * This will use the UI shim to create ParameterCouplings between the faust dsp and iplug iControls
     * However they will not be registered to the daw yet, since loading a preset will need them to claim
     * the right ones so the automation will affect the correct parameters
     */
    UI faustUi(&shared.parameters, &shared.meters);

    buildUserInterface(&faustUi);
    init(pSamplerate);
    if (mType == DEFAULT_NODE_NAME) {
      mType = faustUi.name;
    }


    for (int i = 0, pos = 0; i < shared.parameters.GetSize(); i++) {
      ParameterCoupling* p = shared.parameters.Get(i);
      if (strncmp(p->name, "Stereo", 32) == 0) {
        //continue;
      }
      p->y = p->h * pos - 80;
      pos++;
    }
  }

  void OnChannelsChanged(const int pChannels) override {
    if (mBuffersOut != nullptr) {
      WDBGMSG("Warning trying to change the channelcount on a faust node!\n");
    }
    else {
      Node::OnChannelsChanged(pChannels);
    }
  }

  void OnSamplerateChanged(const int pSamplerate) override {
    instanceConstants(pSamplerate);
  }

  void OnTransport() override {
    instanceClear();
  }

  /**
   * The faust uses a fairly similar way of processing blocks, however
   * A node might have multiple inputs so the right ones have to be forwarded
   */
  virtual void ProcessBlock(const int nFrames) {
    if (!inputsReady() || mIsProcessed || byPass()) { return; }
    for (int i = 1; i < shared.parameters.GetSize(); i++) {
      shared.parameters.Get(i)->update();
    }
    compute(nFrames, shared.socketsIn[0]->mConnectedTo->mParentBuffer, mBuffersOut[0]);
    mIsProcessed = true;
  }
};
