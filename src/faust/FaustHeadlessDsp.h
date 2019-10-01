#ifndef FAUSTHEADLESSDSP
#define FAUSTHEADLESSDSP

#define FAUSTFLOAT iplug::sample

#include "IPlugConstants.h"
#include <map>


struct Meta
{
  virtual void declare(const char* key, const char* value) = 0;
};



struct UI
{
  std::map<const char*, FAUSTFLOAT*> properties;
  virtual void openVerticalBox(const char* key) {};
  virtual void closeBox() {};
  virtual void addHorizontalSlider(const char* name, FAUSTFLOAT* proprety, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT) {
    properties.insert(std::pair<const char*, FAUSTFLOAT*>(name, proprety));
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
