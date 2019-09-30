#ifndef FAUSTHEADLESSDSP
#define FAUSTHEADLESSDSP

#define FAUSTFLOAT iplug::sample

#include "IPlugConstants.h"


struct Meta
{
  virtual void declare(const char* key, const char* value) = 0;
};



struct UI
{
  virtual void openVerticalBox(const char* key) {};
  virtual void closeBox() {};
  virtual void addHorizontalSlider(const char*, FAUSTFLOAT*, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT) {
    
  }
};



struct FaustHeadlessDsp {

  UI ui;
  virtual void init(int samplingFreq) = 0;
  virtual void buildUserInterface(UI* ui_interface) = 0;
  
  void setup(int samplingFreq) {
    buildUserInterface(&ui);
    init(samplingFreq);
  }

  FAUSTFLOAT* getProperty() {
    return 0;
  }

};

#endif 
