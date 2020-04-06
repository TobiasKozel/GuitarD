#pragma once

#include <cassert>
#include <cmath>
#include <algorithm>
#include "./soundwooferTypes.h"

namespace soundwoofer {
  /**
   * Fairly inieficent and low quality resampler
   * No dependecies though
   */
  class Resampler {
    double mStepSize = 0; // Used to step over the input signal, will be > 1 if downsampling
    size_t mWindowSizeOut = 0; // The window size adjusted to the out sample rate
    size_t mMaxWindowSize = 0;
    int mSampleRateIn = 0;
    int mSampleRateOut = 0;

    /**
     * This lowpass filter was generated from the faust butterworth low pass of the 24th order
     */
    void lowPass(float* buffer, int count, int fSampleRate, float cutoff);

    void sincInterpolation(size_t outSamplesPadded, size_t inSamples, float* inBufferPadded, float* outBufferPadded, size_t windowSize) const;

    void linearInterpolation(size_t outSamplesPadded, float* inBufferPadded, float* outBufferPadded);

  public:
    Resampler(const double inFreq, const double outFreq, size_t maxWindowSize = 128);

    /**
     * Does the reampling and allocates a new buffer to store the result in
     * Not realtime safe at all
     */
    size_t resample(float* inBuffer = nullptr, const size_t inSamples = 0, float** outBuffer = nullptr);

    static double sinc(const double i) {
      if (std::abs(i) < 0.00001) {
        return 1.0;
      }
      return (sin(PI * i) / (PI * i));
    }
  };
}

#ifdef SOUNDWOOFER_IMPL
  #include "./soundwooferResamplerImpl.h"
#endif