#ifndef FAUSTHEADLESSDSP
#define FAUSTHEADLESSDSP

#define FAUSTFLOAT iplug::sample

#include <map>
#include <vector>
#include "IPlugConstants.h"
#include "src/logger.h"
#include "src/graph/misc/ParameterCoupling.h"
#include "src/graph/Node.h"


struct Meta {
  virtual void declare(const char* key, const char* value) = 0;
};

/** 
 * This is a shim to collect pointers to all the properties/parameters from the faust DSP code
 */
struct UI {
  std::vector<ParameterCoupling*> params;
  const char* name;

  UI() {
    name = DefaultNodeName;
  }

  void openVerticalBox(const char* key) {
    // NOTE This only is the name of the module if it has one box!
    if (name != DefaultNodeName) {
      WDBGMSG("openVerticalBox called multiple times. The node Type might be wrong!");
      assert(false);
    }
    name = key;
  };
  void openHorizontalBox(const char* key) {};
  void closeBox() {};
  void declare(FAUSTFLOAT*, const char*, const char*) {};

  void addHorizontalSlider(const char* name, FAUSTFLOAT* prop, FAUSTFLOAT p_default, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT stepSize) {
    // For every new there should be a delete eh?
    // Well these will get cleaned up in the node (hopefully)
    params.push_back(new ParameterCoupling(name, prop, p_default, min, max, stepSize));
  }

  void addVerticalSlider(const char* name, FAUSTFLOAT* prop, FAUSTFLOAT p_default, FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT stepSize) {
    params.push_back(new ParameterCoupling(name, prop, p_default, min, max, stepSize));
  }

  void addCheckButton(const char* name, FAUSTFLOAT* prop) {
    params.push_back(new ParameterCoupling(name, prop, 0, 0, 1, 1));
  }

  void addHorizontalBargraph(const char* name, FAUSTFLOAT* value, FAUSTFLOAT min, FAUSTFLOAT max) {};

};


/**
 * The faust DSP code will derive from this
 */
class FaustHeadlessDsp: public Node {
public:
  UI ui;

  // These three will be overridden by the generated faust code
  virtual void init(int samplingFreq) = 0;
  virtual void buildUserInterface(UI* ui_interface) = 0;
  virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) = 0;
  virtual int getNumInputs() = 0;
  virtual int getNumOutputs() = 0;

  void setup(int p_samplerate = 48000, int p_maxBuffer = 512, int p_channles = 2, int p_inputs = 1, int p_outputs = 1) {
    Node::setup(p_samplerate, p_maxBuffer, p_channles, getNumInputs() / p_channles, getNumOutputs() / p_channles);
    
    /**
     * This will use the UI shim to create ParameterCouplings between the faust dsp and iplug iControls
     * However they will not be registered to the daw yet, since loading a preset will need them to claim
     * the right ones so the automation will affect the correct parameters
     */
    buildUserInterface(&ui);
    init(p_samplerate);

    if (type == DefaultNodeName) {
      type = ui.name;
    }

    // Keep the pointers around in a normal array since this might be faster than iterating over a vector
    parameters = new ParameterCoupling * [ui.params.size()];
    parameterCount = 0;
    for (auto p : ui.params) {
      parameters[parameterCount] = p;
      p->y = p->h * parameterCount;
      parameterCount++;
    }
  }

  /**
   * The faust uses a fairly similar way of processing blocks, however
   * A node might have multiple inputs so the right ones have to be forwarded
   */
  virtual void ProcessBlock(int nFrames) {
    if (!inputsReady()) { return; }

    for (int i = 0; i < parameterCount; i++) {
      parameters[i]->update();
    }
    compute(nFrames, inputs[0]->outputs[0], outputs[0]);
    isProcessed = true;
  }
};

#endif 
