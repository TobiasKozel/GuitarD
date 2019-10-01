#ifndef FAUSTHEADLESSDSP
#define FAUSTHEADLESSDSP

#define FAUSTFLOAT iplug::sample

#include "IPlugConstants.h"
#include <map>


struct Meta
{
  virtual void declare(const char* key, const char* value) = 0;
};


/** 
 * This is a shim to collect pointer to all the properties from the faust DSP code
 * It will store them in a map for easy access by property name
*/
struct UI
{
  std::map<const char*, FAUSTFLOAT*> properties;
  virtual void openVerticalBox(const char* key) {};
  virtual void openHorizontalBox(const char* key) {};
  virtual void closeBox() {};
  virtual void declare(FAUSTFLOAT*, const char*, const char*) {};

  virtual void addHorizontalSlider(const char* name, FAUSTFLOAT* proprety, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT) {
    properties.insert(std::pair<const char*, FAUSTFLOAT*>(name, proprety));
  }

  virtual void addVerticalSlider(const char* name, FAUSTFLOAT* proprety, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT, FAUSTFLOAT) {
    properties.insert(std::pair<const char*, FAUSTFLOAT*>(name, proprety));
  }

  virtual void addCheckButton(const char* name, FAUSTFLOAT* proprety) {
    properties.insert(std::pair<const char*, FAUSTFLOAT*>(name, proprety));
  }

  virtual void addHorizontalBargraph(const char* name, FAUSTFLOAT* value, FAUSTFLOAT min, FAUSTFLOAT max) {};

  FAUSTFLOAT* getProperty(const char* name) {
    if (properties.find(name) != properties.end()) {
      return properties.at(name);
    }
    else {
      return nullptr;
    }
  }
};


/**
 * The faust DSP code will derive from this
*/
struct FaustHeadlessDsp {
public:
  UI ui;
private:
  virtual void init(int samplingFreq) = 0;
  virtual void buildUserInterface(UI* ui_interface) = 0;

public:
  /**
   * This should be used to init the DSP code since it will also gather all the properties
  */
  void setup(int samplingFreq) {
    buildUserInterface(&ui);
    init(samplingFreq);
  }

  FAUSTFLOAT* getProperty(const char* name) {
    return ui.getProperty(name);
  }

};

#endif 
