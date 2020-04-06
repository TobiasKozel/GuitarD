#pragma once

#include "./soundwooferResampler.h"

namespace soundwoofer {
  void Resampler::lowPass(float* buffer, int count, int fSampleRate, float cutoff) {
    float fConst0;
    float fVslider0 = cutoff;
    float fRec11[3];
    float fRec10[3];
    float fRec9[3];
    float fRec8[3];
    float fRec7[3];
    float fRec6[3];
    float fRec5[3];
    float fRec4[3];
    float fRec3[3];
    float fRec2[3];
    float fRec1[3];
    float fRec0[3];
    fConst0 = (3.14159274f / std::min<float>(192000.0f, std::max<float>(1.0f, float(fSampleRate))));
    for (int l0 = 0; (l0 < 3); l0 = (l0 + 1)) {
      fRec11[l0] = 0.0f;
    }
    for (int l1 = 0; (l1 < 3); l1 = (l1 + 1)) {
      fRec10[l1] = 0.0f;
    }
    for (int l2 = 0; (l2 < 3); l2 = (l2 + 1)) {
      fRec9[l2] = 0.0f;
    }
    for (int l3 = 0; (l3 < 3); l3 = (l3 + 1)) {
      fRec8[l3] = 0.0f;
    }
    for (int l4 = 0; (l4 < 3); l4 = (l4 + 1)) {
      fRec7[l4] = 0.0f;
    }
    for (int l5 = 0; (l5 < 3); l5 = (l5 + 1)) {
      fRec6[l5] = 0.0f;
    }
    for (int l6 = 0; (l6 < 3); l6 = (l6 + 1)) {
      fRec5[l6] = 0.0f;
    }
    for (int l7 = 0; (l7 < 3); l7 = (l7 + 1)) {
      fRec4[l7] = 0.0f;
    }
    for (int l8 = 0; (l8 < 3); l8 = (l8 + 1)) {
      fRec3[l8] = 0.0f;
    }
    for (int l9 = 0; (l9 < 3); l9 = (l9 + 1)) {
      fRec2[l9] = 0.0f;
    }
    for (int l10 = 0; (l10 < 3); l10 = (l10 + 1)) {
      fRec1[l10] = 0.0f;
    }
    for (int l11 = 0; (l11 < 3); l11 = (l11 + 1)) {
      fRec0[l11] = 0.0f;
    }
    float* input0 = buffer;
    float* output0 = buffer;
    float fSlow0 = std::tan((fConst0 * float(fVslider0)));
    float fSlow1 = (1.0f / fSlow0);
    float fSlow2 = (1.0f / (((fSlow1 + 0.130806252f) / fSlow0) + 1.0f));
    float fSlow3 = (1.0f / (((fSlow1 + 0.390180647f) / fSlow0) + 1.0f));
    float fSlow4 = (1.0f / (((fSlow1 + 0.64287895f) / fSlow0) + 1.0f));
    float fSlow5 = (1.0f / (((fSlow1 + 0.884577394f) / fSlow0) + 1.0f));
    float fSlow6 = (1.0f / (((fSlow1 + 1.11114049f) / fSlow0) + 1.0f));
    float fSlow7 = (1.0f / (((fSlow1 + 1.31869161f) / fSlow0) + 1.0f));
    float fSlow8 = (1.0f / (((fSlow1 + 1.50367963f) / fSlow0) + 1.0f));
    float fSlow9 = (1.0f / (((fSlow1 + 1.66293919f) / fSlow0) + 1.0f));
    float fSlow10 = (1.0f / (((fSlow1 + 1.79374552f) / fSlow0) + 1.0f));
    float fSlow11 = (1.0f / (((fSlow1 + 1.89386022f) / fSlow0) + 1.0f));
    float fSlow12 = (1.0f / (((fSlow1 + 1.9615705f) / fSlow0) + 1.0f));
    float fSlow13 = (1.0f / (((fSlow1 + 1.99571788f) / fSlow0) + 1.0f));
    float fSlow14 = (((fSlow1 + -1.99571788f) / fSlow0) + 1.0f);
    float fSlow15 = (2.0f * (1.0f - (1.0f / (fSlow0 * fSlow0))));
    float fSlow16 = (((fSlow1 + -1.9615705f) / fSlow0) + 1.0f);
    float fSlow17 = (((fSlow1 + -1.89386022f) / fSlow0) + 1.0f);
    float fSlow18 = (((fSlow1 + -1.79374552f) / fSlow0) + 1.0f);
    float fSlow19 = (((fSlow1 + -1.66293919f) / fSlow0) + 1.0f);
    float fSlow20 = (((fSlow1 + -1.50367963f) / fSlow0) + 1.0f);
    float fSlow21 = (((fSlow1 + -1.31869161f) / fSlow0) + 1.0f);
    float fSlow22 = (((fSlow1 + -1.11114049f) / fSlow0) + 1.0f);
    float fSlow23 = (((fSlow1 + -0.884577394f) / fSlow0) + 1.0f);
    float fSlow24 = (((fSlow1 + -0.64287895f) / fSlow0) + 1.0f);
    float fSlow25 = (((fSlow1 + -0.390180647f) / fSlow0) + 1.0f);
    float fSlow26 = (((fSlow1 + -0.130806252f) / fSlow0) + 1.0f);
    for (int i = 0; (i < count); i = (i + 1)) {
      fRec11[0] = (float(input0[i]) - (fSlow13 * ((fSlow14 * fRec11[2]) + (fSlow15 * fRec11[1]))));
      fRec10[0] = ((fSlow13 * (fRec11[2] + (fRec11[0] + (2.0f * fRec11[1])))) - (fSlow12 * ((fSlow16 * fRec10[2]) + (fSlow15 * fRec10[1]))));
      fRec9[0] = ((fSlow12 * (fRec10[2] + (fRec10[0] + (2.0f * fRec10[1])))) - (fSlow11 * ((fSlow17 * fRec9[2]) + (fSlow15 * fRec9[1]))));
      fRec8[0] = ((fSlow11 * (fRec9[2] + (fRec9[0] + (2.0f * fRec9[1])))) - (fSlow10 * ((fSlow18 * fRec8[2]) + (fSlow15 * fRec8[1]))));
      fRec7[0] = ((fSlow10 * (fRec8[2] + (fRec8[0] + (2.0f * fRec8[1])))) - (fSlow9 * ((fSlow19 * fRec7[2]) + (fSlow15 * fRec7[1]))));
      fRec6[0] = ((fSlow9 * (fRec7[2] + (fRec7[0] + (2.0f * fRec7[1])))) - (fSlow8 * ((fSlow20 * fRec6[2]) + (fSlow15 * fRec6[1]))));
      fRec5[0] = ((fSlow8 * (fRec6[2] + (fRec6[0] + (2.0f * fRec6[1])))) - (fSlow7 * ((fSlow21 * fRec5[2]) + (fSlow15 * fRec5[1]))));
      fRec4[0] = ((fSlow7 * (fRec5[2] + (fRec5[0] + (2.0f * fRec5[1])))) - (fSlow6 * ((fSlow22 * fRec4[2]) + (fSlow15 * fRec4[1]))));
      fRec3[0] = ((fSlow6 * (fRec4[2] + (fRec4[0] + (2.0f * fRec4[1])))) - (fSlow5 * ((fSlow23 * fRec3[2]) + (fSlow15 * fRec3[1]))));
      fRec2[0] = ((fSlow5 * (fRec3[2] + (fRec3[0] + (2.0f * fRec3[1])))) - (fSlow4 * ((fSlow24 * fRec2[2]) + (fSlow15 * fRec2[1]))));
      fRec1[0] = ((fSlow4 * (fRec2[2] + (fRec2[0] + (2.0f * fRec2[1])))) - (fSlow3 * ((fSlow25 * fRec1[2]) + (fSlow15 * fRec1[1]))));
      fRec0[0] = ((fSlow3 * (fRec1[2] + (fRec1[0] + (2.0f * fRec1[1])))) - (fSlow2 * ((fSlow26 * fRec0[2]) + (fSlow15 * fRec0[1]))));
      output0[i] = float((fSlow2 * (fRec0[2] + (fRec0[0] + (2.0f * fRec0[1])))));
      fRec11[2] = fRec11[1];
      fRec11[1] = fRec11[0];
      fRec10[2] = fRec10[1];
      fRec10[1] = fRec10[0];
      fRec9[2] = fRec9[1];
      fRec9[1] = fRec9[0];
      fRec8[2] = fRec8[1];
      fRec8[1] = fRec8[0];
      fRec7[2] = fRec7[1];
      fRec7[1] = fRec7[0];
      fRec6[2] = fRec6[1];
      fRec6[1] = fRec6[0];
      fRec5[2] = fRec5[1];
      fRec5[1] = fRec5[0];
      fRec4[2] = fRec4[1];
      fRec4[1] = fRec4[0];
      fRec3[2] = fRec3[1];
      fRec3[1] = fRec3[0];
      fRec2[2] = fRec2[1];
      fRec2[1] = fRec2[0];
      fRec1[2] = fRec1[1];
      fRec1[1] = fRec1[0];
      fRec0[2] = fRec0[1];
      fRec0[1] = fRec0[0];
    }
  }

  void Resampler::sincInterpolation(size_t outSamplesPadded, size_t inSamples, float* inBufferPadded, float* outBufferPadded, size_t windowSize) const {
    // Windowing only seemed to improve the quality for bigger windows
    // double* window = new double[windowSize];
    // for (size_t i = 0; i < windowSize; i++) { // Use a raised cos as a window
    //     // window[i] = -cos(i * (1.0 / windowSize) * 2 * PI) * 0.5 + 0.5;
    //     window[i] = 1;
    //     // window[i] = sin(i * (1.0 / windowSize) * PI);
    // }
    double windowHalf = windowSize * 0.5;
    /**
     * The position at which the sinc interpolation of the input signal should be evaluated
     * if this is not a double, we'll get some jumps when rounding at the wrong time
     */
    double j = 0.0;
    for (size_t k = 0; k < outSamplesPadded; k++, j += mStepSize) {
      double interpolated = 0;
      const size_t lower = std::floor(std::max(0.0, j - windowHalf));
      const size_t upper = std::ceil(std::min(inSamples * 1.0, j + windowHalf));
      for (size_t i = lower; i < upper; i++) {
        interpolated += inBufferPadded[i] * sinc(j - i) /** * mWindow[i - lower] */;
      }
      outBufferPadded[k] = interpolated;
    }
    // delete[] window;
  }

  void Resampler::linearInterpolation(size_t outSamplesPadded, float* inBufferPadded, float* outBufferPadded) {
    for (size_t i = 0; i < outSamplesPadded; i++) {
      double inter = i * mStepSize;
      const size_t lower = std::floor(inter);
      const double a = inter - lower;
      outBufferPadded[i] = (inBufferPadded[lower] * (1 - a) + inBufferPadded[lower + 1] * a);
    }
  }

  Resampler::Resampler(const double inFreq, const double outFreq, size_t maxWindowSize) {
    mStepSize = inFreq / outFreq; // the step size in the input signal
    mSampleRateIn = inFreq;
    mSampleRateOut = outFreq;
    if (mStepSize < 0.1 || 10 < mStepSize) { // Limit to 10x resampling factor
      mStepSize = 1.0;
      assert(false);
    }
    mMaxWindowSize = maxWindowSize;
    mWindowSizeOut = maxWindowSize / mStepSize;
  }

  size_t Resampler::resample(float* inBuffer, const size_t inSamples, float** outBuffer) {
    const size_t outSamples = std::ceil(inSamples / mStepSize);
    if (outSamples <= 0) { return 0; }

    // Create the output buffer
    const size_t outSamplesPadded = outSamples + mWindowSizeOut;
    float* outBufferPadded = new float[outSamplesPadded]; // Make the output buffer, also allow space for the window
    

    // Create a input buffer and copy the samples over
    const size_t inSamplesPadded = inSamples + mMaxWindowSize;
    float* inBufferPadded = new float[inSamplesPadded];
    ::memset(inBufferPadded, 0, inSamplesPadded * sizeof(float));
    ::memcpy(inBufferPadded + mMaxWindowSize, inBuffer, inSamples * sizeof(float)); // copy in the buffer with padding at the front

    if (mSampleRateOut < mSampleRateIn) {
      // Downsampling means we'll need to lowpass first to get rid of everything exceeding the new bandlimit
      lowPass(inBufferPadded, inSamplesPadded, mSampleRateIn, mSampleRateOut * 0.5 * 0.85);

      if (fmod(mStepSize, 0.25) == 0) { // even ratios will work well with linear interpolations
        linearInterpolation(outSamplesPadded, inBufferPadded, outBufferPadded);
      }
      else {
        sincInterpolation(outSamplesPadded, inSamples, inBufferPadded, outBufferPadded, mMaxWindowSize);
      }
    }
    else {
      // upsampling means, we'll do the interpolation first
      if (fmod(mStepSize, 0.25) == 0) { // even ratios will work well with linear interpolations
        linearInterpolation(outSamplesPadded, inBufferPadded, outBufferPadded);
      }
      else {
        sincInterpolation(outSamplesPadded, inSamples, inBufferPadded, outBufferPadded, mMaxWindowSize * 0.25);
      }
      lowPass(outBufferPadded, outSamplesPadded, mSampleRateOut, mSampleRateIn * 0.5);
    }
    ::memmove(outBufferPadded, outBufferPadded + mWindowSizeOut, outSamples * sizeof(float));
    delete[] inBufferPadded;
    (*outBuffer) = outBufferPadded;
    return outSamples;
  }



}