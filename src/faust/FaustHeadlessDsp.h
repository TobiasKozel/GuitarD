#ifndef FAUSTHEADLESSDSP
#define FAUSTHEADLESSDSP

#define FAUSTFLOAT iplug::sample

#include "IPlugConstants.h"
#include <map>
#include <vector>
#include "src/graph/ParameterManager.h"


struct Meta {
  virtual void declare(const char* key, const char* value) = 0;
};

/** 
 * This is a shim to collect pointers to all the properties/parameters from the faust DSP code
 */
struct UI {
  std::vector<ParameterCoupling*> params;

  void openVerticalBox(const char* key) {};
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

  ~UI() {
  }
};


/**
 * The faust DSP code will derive from this
 */
struct FaustHeadlessDsp {
public:
  UI ui;
  virtual void init(int samplingFreq) = 0;
  virtual void buildUserInterface(UI* ui_interface) = 0;

  /**
   * This should be used to init the DSP code since it will also gather all the properties
   */
  void setup(int samplingFreq) {
    buildUserInterface(&ui);
    init(samplingFreq);
  }
};

#endif 
