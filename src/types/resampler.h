#pragma once
#include <math.h>
#include "./types.h"

namespace guitard {
  template <typename Tin, typename Tout>
  class SincResampler {
    double mStepSize = 0;
  public:
    SincResampler(const double inFreq, const double outFreq) {
      mStepSize = inFreq / outFreq;
    }

    /**
     * Will allocate an array for you with the correct size
     * This is extremely slow since it will take all the samples into account for resampling
     */
    size_t resample(Tin* in, const size_t length, Tout** out, Tout amplitude = 1.0) {
      const size_t outSamples = length / mStepSize;
      if (outSamples <= 0) { return 0; }
      (*out) = new Tout[outSamples];
      double j = 0; // The position at which the sinc interpolation of the input signal should be evaluated
      for (size_t k = 0; k < outSamples; k++, j += mStepSize) {
        Tout interpolated = 0;
        for (size_t i = 0; i < length; i++) {
          interpolated += in[i] * sinc(j - i);
        }
        (*out)[k] = interpolated * amplitude;
      }
      return outSamples;
    }

    static Tout sinc(const double i) {
      if (i == 0) {
        return 1;
      }
      return (sin(PI * i) / (PI * i));
    }
  };

  template <typename Tin, typename Tout>
  class WindowedSincResampler {
    double mStepSize = 0;
    Tout* mSincTable = nullptr;
    size_t mWindowSize = 0;
  public:
    WindowedSincResampler(const double inFreq, const double outFreq, const size_t windowSize = 24) {
      mStepSize = inFreq / outFreq; // the step size in the input signal
      mWindowSize = windowSize;
      mSincTable = new Tout[windowSize];
      double sincIndex = (-windowSize / 2.f) / mStepSize;
      const double sincStep = 1.f / mStepSize;
      for (size_t i = 0; i < windowSize; i++) {
        mSincTable[i] = sinc(sincIndex);
        sincIndex += sincStep;
      }
      //for (size_t i = 0; i < blockSize; i++) {
      //  mWindow[i] = cos((i * 0.5 * PI) / static_cast<double>(blockSize));
      //}
    }

    ~WindowedSincResampler() {
      delete mSincTable;
    }

    size_t resample(Tin* in, const size_t length, Tout** out, Tout amplitude = 1.0) {
      const size_t outSamples = length / mStepSize;
      if (outSamples <= 0) { return 0; }
      (*out) = new Tout[outSamples];
      double j = 0; // The position at which the sinc interpolation of the input signal should be evaluated
      for (size_t k = 0; k < outSamples; k++, j += mStepSize) {
        Tout interpolated = 0;
        const size_t lower = std::max(static_cast<size_t>(0), static_cast<size_t>(j - (mWindowSize / 2)));
        const size_t upper = std::min(length, static_cast<size_t>(j + mWindowSize / 2));
        for (size_t i = lower; i < upper; i++) {
          interpolated += in[i] * sinc(j - i);
        }
        (*out)[k] = interpolated * amplitude;
      }
      return outSamples;
    }

    static Tout lerp(const double i, Tin* buf) {
      const size_t lower = std::floor(i);
      const double a = i - lower;
      return (buf[lower] * (1 - a) + buf[lower + 1] * a);
    }

    static Tout sinc(const double i) {
      if (i == 0) {
        return 1;
      }
      return (sin(PI * i) / (PI * i));
    }
  };

  template <typename Tin, typename Tout>
  class LinearResampler {
    double mStepSize = 0;
    bool mEqual = false;
  public:
    LinearResampler(const double inFreq, const double outFreq) {
      mStepSize = inFreq / outFreq;
      mEqual = std::abs(inFreq - outFreq) < 0.01;
    }

    /**
     * Will allocate an array for you with the correct size
     * This is a super basic resampler and shouldn't really be used
     */
    size_t resample(Tin* in, const size_t length, Tout** out, Tout amplitude = 1.0) {
      const size_t outSamples = length / mStepSize;
      if (outSamples <= 0) { return 0; }
      (*out) = new Tout[outSamples];
      for (size_t i = 0; i < outSamples; i++) {
        double inter = i * mStepSize;
        const size_t lower = std::floor(inter);
        const double a = inter - lower;
        (*out)[i] = (in[lower] * (1 - a) + in[lower + 1] * a) * amplitude;
      }
      return outSamples;
    }
  };
}