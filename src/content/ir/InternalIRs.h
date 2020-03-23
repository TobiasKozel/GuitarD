#pragma once

#include "../../../thirdparty/soundwoofer/soundwoofer.h"
#include "./clean.ir"
#include "./air.ir"
namespace guitard {
  soundwoofer::SWImpulseShared InternalIRs[] = {
    soundwoofer::ir::createGeneric("Clean", InteralIR::cleanIR, InteralIR::cleanIRLength, 1, 48000, soundwoofer::EMBEDDED_SRC),
    soundwoofer::ir::createGeneric("Air", InteralIR::airIR, InteralIR::airIRLength, 1, 48000, soundwoofer::EMBEDDED_SRC)
  };

  int InternalIRsCount = 2;
}