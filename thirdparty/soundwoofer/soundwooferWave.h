#pragma once

namespace soundwoofer {
  namespace wave {
    template <typename Tin, typename Tout>
    /**
     * Simple since resampler, still needs a lowpass to avoid aliasing
     */
    class WindowedSincResampler {
      double mStepSize = 0;
      size_t mWindowSize = 0;
    public:
      WindowedSincResampler(const double inFreq, const double outFreq, const size_t windowSize = 24) {
        mStepSize = inFreq / outFreq; // the step size in the input signal
        mWindowSize = windowSize;
      }

      ~WindowedSincResampler() {
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
  }
}