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
  NodeShared* shared;
  const char* name;

  UI(NodeShared* data) {
    shared = data;
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
    shared->parameters[shared->parameterCount] = new ParameterCoupling(name, prop, pDefault, min, max, stepSize);
    shared->parameterCount++;
  }

  void addVerticalSlider(const char* name, FAUSTFLOAT* prop, FAUSTFLOAT pDefault, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT stepSize) const {
    addHorizontalSlider(name, prop, pDefault, min, max, stepSize);
  }

  void addCheckButton(const char* name, FAUSTFLOAT* prop) const {
    addHorizontalSlider(name, prop, 0, 0, 1, 1);
  }

  void addVerticalBargraph(const char* name, FAUSTFLOAT* prop, FAUSTFLOAT min, FAUSTFLOAT max) const {
    // They never get initialized in the Faust code
    *prop = 0;
    shared->meters[shared->meterCount] = new MeterCoupling{ prop, name, min, max };
    shared->meterCount++;
  };

  void addHorizontalBargraph(const char* name, FAUSTFLOAT* prop, FAUSTFLOAT min, FAUSTFLOAT max) const {
    addHorizontalBargraph(name, prop, min, max);
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
    UI faustUi(&shared);

    buildUserInterface(&faustUi);
    init(pSamplerate);
    if (shared.type == DEFAULT_NODE_NAME) {
      shared.type = faustUi.name;
    }

    const int perColumn = 2;
    const int columns = ceil(shared.parameterCount / static_cast<float>(perColumn));

    for (int i = 0, pos = 0; i < shared.parameterCount; i++) {
      const int column = pos / perColumn;
      ParameterCoupling* p = shared.parameters[i];
      if (strncmp(p->name, "Bypass", 32) == 0) {
        continue;
      }
      p->x = column * 60 + 50 - shared.width * 0.5;
      p->y = 60 * (pos % perColumn) - 40;
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
    for (int i = 1; i < shared.parameterCount; i++) {
      shared.parameters[i]->update();
    }
    compute(nFrames, shared.socketsIn[0]->mConnectedTo[0]->mParentBuffer, mBuffersOut[0]);
    mIsProcessed = true;
  }
};
