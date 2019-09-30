#ifndef FAUSTHEADLESSDSP
#define FAUSTHEADLESSDSP

#include "IPlugConstants.h"


struct Meta
{
  virtual void declare(const char* key, const char* value) = 0;
};

struct UI
{
  virtual void openVerticalBox(const char* key) = 0;
  virtual void closeBox() = 0;
};



class FaustHeadlessDsp {

};

#endif 
