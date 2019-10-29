#pragma once
#include "OverDrive.h"

class OverDriveNode : public OverDrive {
public:
  OverDriveNode(std::string pType) {
    type = pType;
  }
};