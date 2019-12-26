#pragma once

#include "src/misc/GStructs.h"
#include "clean.h"
#include "air.h"
#include "wdlstring.h"

IRBundle InternalIRs[] = {
  { WDL_String("Clean"), 1, 48000, InteralIR::cleanIRLength, InteralIR::cleanIR, nullptr },
  { WDL_String("Air"), 1, 48000, InteralIR::airIRLength, InteralIR::airIR, nullptr }
};

int InternalIRsCount = 2;
