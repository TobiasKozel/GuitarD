#pragma once

#include "./soundwooferTypes.h"

namespace soundwoofer {
  namespace wave {
    void normalizeIR(SWImpulseShared& ir);

#ifndef SOUNDWOOFER_CUSTOM_WAVE
    /**
     * decodes, deinterleaves and resamples the wave
     */
    Status decodeWave(SWImpulseShared& ir, void* dwav, size_t sampleRate, bool normalize);

    /**
     * Loads a wave file from an absolute path
     */
    Status loadWaveFile(SWImpulseShared& ir, std::string absolutePath, size_t sampleRate, bool normalize);

    /**
     * Loads a wave from memory
     */
    Status loadWaveMemory(SWImpulseShared& ir, const char* waveData, const size_t length, size_t sampleRate, bool normalize);
#endif
  }
}

#ifdef SOUNDWOOFER_IMPL
  #include "./soundwooferWaveImpl.h"
#endif