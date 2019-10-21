#pragma once
#include "CryBaby.h"

class CryBabyNode : public CryBaby {
public:
  CryBabyNode(std::string pType) {
    type = pType;
  }
  //parameters[0]->y = 100;
  //parameters[0]->x = 100;
};