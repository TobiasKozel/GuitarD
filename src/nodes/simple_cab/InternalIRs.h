#pragma once

#include "src/types/gstructs.h"
#include "clean.h"
#include "air.h"
#include "wdlstring.h"
namespace guitard {
  IRBundle InternalIRs[] = {
    { "Clean", 1, 48000, InteralIR::cleanIRLength, InteralIR::cleanIR },
    { "Air", 1, 48000, InteralIR::airIRLength, InteralIR::airIR }
  };

  int InternalIRsCount = 2;
}