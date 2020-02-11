#pragma once

#include "../../../thirdparty/soundwoofer/soundwoofer.h"
#include "./clean.h"
#include "./air.h"
namespace guitard {
  SoundWoofer::SWImpulseShared InternalIRs[] = {
    SoundWoofer::createGenericIR("Clean", InteralIR::cleanIR, InteralIR::cleanIRLength, 1, 48000, SoundWoofer::EMBEDDED_SRC),
    SoundWoofer::createGenericIR("Air", InteralIR::airIR, InteralIR::airIRLength, 1, 48000, SoundWoofer::EMBEDDED_SRC)
  };

  int InternalIRsCount = 0;
}