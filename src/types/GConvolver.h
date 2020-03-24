#pragma once
//#define GUITARD_CONV_THREAD_POOL
//#define GUITARD_CONV_OPENMP

#ifdef GUITARD_CONV_OPENMP
  #include <omp.h>
#endif

#include "../GConfig.h"
#include "./GTypes.h"

/**
 * Decide which type the convolution will use
 */
#ifndef GUITARD_FLOAT_CONVOLUTION
  #define FFTCONVOLVER_TYPE guitard::sample
#else
  /**
   * If it's float the convolver can do sse
   */
  #define FFTCONVOLVER_TYPE float

  #ifdef GUITARD_SSE
    #define FFTCONVOLVER_USE_SSE
  #endif
#endif

/**
 * Figure out whether there needs to be a conversion
 */
#if defined(GUITARD_FLOAT_CONVOLUTION) && defined(SAMPLE_TYPE_FLOAT) || !defined(GUITARD_FLOAT_CONVOLUTION) && !defined(SAMPLE_TYPE_FLOAT)
  #define GUITARD_CONV_SAME_TYPE
#endif

#include "../../thirdparty/convolver/twoStageConvolver.h"

#ifdef GUITARD_CONV_THREAD_POOL
  #include "../../thirdparty/threadpool.h"
#endif

namespace guitard {
  /**
   * Wraps up the FttConvolver to do easy stereo convolution
   * and also deal with the buffers
   */
  class WrappedConvolver {
    const int CONV_BLOCK_SIZE = 128;
    const int CONV_TAIL_BLOCK_SIZE = 1024 * 4;
    static const int CHANNEL_COUNT = 2;
    int mMaxBuffer = 0;

#ifdef GUITARD_CONV_THREAD_POOL
    ctpl::thread_pool mPool;
#endif

    /** We'll only do stereo convolution at most */
    fftconvolver::TwoStageFFTConvolver mConvolvers[CHANNEL_COUNT];

#ifndef GUITARD_CONV_SAME_TYPE
    /** Buffers need to be converted from double to float */
    FFTCONVOLVER_TYPE mConversionBufferIn[CHANNEL_COUNT][GUITARD_MAX_BUFFER];
    FFTCONVOLVER_TYPE mConversionBufferOut[CHANNEL_COUNT][GUITARD_MAX_BUFFER];
#endif

    bool mIRLoaded = false;
    const int maxBuffer;
    bool mIsProcessing = false;
  public:

    bool mStereo = false;

    GUITARD_NO_COPY(WrappedConvolver)

    explicit WrappedConvolver(const int maxBuffer = 512): maxBuffer(maxBuffer) {
#ifdef GUITARD_CONV_THREAD_POOL
      mPool.resize(1);
#endif
      mMaxBuffer = maxBuffer;
    }

    void loadIR(float** samples, const size_t sampleCount, const size_t channelCount) {
      if (samples == nullptr || sampleCount == 0 || channelCount == 0) { return; }
      mIRLoaded = false;
      while (mIsProcessing) {}

      for (int c = 0; c < channelCount; c++) {
        if (channelCount == 1) {
          for (int ch = 0; ch < CHANNEL_COUNT; ch++) {
            mConvolvers[ch].init(CONV_BLOCK_SIZE, CONV_TAIL_BLOCK_SIZE, samples[0], sampleCount);
          }
        }
        else if (channelCount == CHANNEL_COUNT) {
          mConvolvers[c].init(CONV_BLOCK_SIZE, CONV_TAIL_BLOCK_SIZE, samples[c], sampleCount);
        }
      }
      mIRLoaded = true;
    }

    void ProcessBlock(sample** in, sample** out, const int nFrames) {

      if (!mIRLoaded) { // kust pass the signal through
        for (int c = 0; c < CHANNEL_COUNT; c++) {
          for (int i = 0; i < nFrames; i++) {
            out[c][i] = in[c][i];
          }
        }
        return;
      }
      
      mIsProcessing = true;

#ifdef GUITARD_CONV_THREAD_POOL
      /**                          THREADPOOL TEST                       */
      std::future<void> right;
      if (mStereo) { // do the conv on the second channel
        right = mPool.push([&](int id) {
          processChannel(in, out, nFrames, 1);
        });
      }

      processChannel(in, out, nFrames, 0);

      if (mStereo) {
        right.wait(); // wait for the second channel result
      }
      else {
        ::memcpy(out[1], out[0], nFrames * sizeof(sample));
      }
#else
#ifdef GUITARD_CONV_OPENMP
      /**                             OPENMP TEST                        */
      if (mStereo) {
#pragma omp parallel num_threads(2)
#pragma omp for
        for (int n = 0; n < 2; ++n) {
          processChannel(in, out, nFrames, n);
        }
      }
      else {
        processChannel(in, out, nFrames, 0);
        ::memcpy(out[1], out[0], nFrames * sizeof(sample));
      }
#else
      /**                           REFERENCE                           */
      processChannel(in, out, nFrames, 0);

      if (!mStereo) { // mono needs the other channel filled too
        ::memcpy(out[1], out[0], nFrames * sizeof(sample));
    }
      else { // else do the second channel as well
        processChannel(in, out, nFrames, 1);
      }
#endif
#endif

      mIsProcessing = false;
    }

    static String getLicense() {
      String l = "Realtime Convolution by\n";
      l += "https://github.com/HiFi-LoFi\n";
      l += "https://github.com/HiFi-LoFi/FFTConvolver\n";
      l += "MIT License\n\n";
      l += "Wave file reader \"dr_wav\"";
      l += "https://github.com/mackron\n";
      l += "https://github.com/mackron/dr_libs/blob/master/dr_wav.h\n";
      l += "Public domain";
      return l;
    }

  private:

    inline void processChannel(sample** in, sample** out, const int nFrames, int channel) {
#ifdef GUITARD_CONV_SAME_TYPE
      mConvolvers[channel].process(in[channel], out[channel], nFrames); // no conversion needed
#else
      for (int i = 0; i < nFrames; i++) {
        mConversionBufferIn[channel][i] = static_cast<float>(in[channel][i]);
      }
      mConvolvers[channel].process(mConversionBufferIn[channel], mConversionBufferOut[channel], nFrames);
      for (int i = 0; i < nFrames; i++) {
        out[channel][i] = mConversionBufferOut[channel][i];
      }
#endif
    }
  };
}