#pragma once
// #define useThreadPool
// #define useOpenMP

#ifdef useOpenMP
  #include <omp.h>
#endif

#include "../../misc/constants.h"
#include "../../types/types.h"
#ifndef GUITARD_FLOAT_CONVOLUTION
  #define FFTCONVOLVER_TYPE guitard::sample
#else
  #define FFTCONVOLVER_TYPE float
#endif

#include "../../../thirdparty/convolver/twoStageConvolver.h"
#include "../../../thirdparty/threadpool.h"
#include "../../types/resampler.h"

namespace guitard {
  class WrappedConvolver {
    const int CONV_BLOCK_SIZE = 128;
    const int CONV_TAIL_BLOCK_SIZE = 1024 * 4;
    static const int CHANNEL_COUNT = 2;
    int mMaxBuffer = 0;
#ifdef useThreadPool
    ctpl::thread_pool tPool;
#endif

    /** We'll only do stereo convolution at most */
    fftconvolver::TwoStageFFTConvolver* mConvolvers[CHANNEL_COUNT] = { nullptr };

    /** Buffers need to be converted from double to float */
    FFTCONVOLVER_TYPE** mConversionBufferIn = nullptr;
    FFTCONVOLVER_TYPE** mConversionBufferOut = nullptr;

    int mSampleRate = 0;

    bool mIRLoaded = false;
    const int maxBuffer;

    // IRBundle* mLoadedIr = nullptr;
  public:

    bool mStereo = false;

    /**
     * Won't allow copying for now
     */
    WrappedConvolver(const WrappedConvolver&) = delete;
    WrappedConvolver(const WrappedConvolver*) = delete;
    WrappedConvolver(WrappedConvolver&&) = delete;
    WrappedConvolver& operator= (const WrappedConvolver&) = delete;
    WrappedConvolver& operator= (WrappedConvolver&&) = delete;

    explicit WrappedConvolver(const int samplerate = 48000, const int maxBuffer = 512): maxBuffer(maxBuffer) {
#ifdef useThreadPool
      tPool.resize(2);
#endif
      mSampleRate = samplerate;
      for (int c = 0; c < CHANNEL_COUNT; c++) {
        mConvolvers[c] = new fftconvolver::TwoStageFFTConvolver();
      }
      mMaxBuffer = maxBuffer;
#ifdef GUITARD_FLOAT_CONVOLUTION
      mConversionBufferIn = new FFTCONVOLVER_TYPE *[CHANNEL_COUNT];
      mConversionBufferOut = new FFTCONVOLVER_TYPE * [CHANNEL_COUNT];
      for (int c = 0; c < CHANNEL_COUNT; c++) {
        mConversionBufferIn[c] = new FFTCONVOLVER_TYPE[mMaxBuffer];
        mConversionBufferOut[c] = new FFTCONVOLVER_TYPE[mMaxBuffer];
      }
#endif
    }

    ~WrappedConvolver() {
      for (int c = 0; c < CHANNEL_COUNT; c++) {
        delete mConvolvers[c];
      }
#ifdef GUITARD_FLOAT_CONVOLUTION
      for (int c = 0; c < CHANNEL_COUNT; c++) {
        delete[] mConversionBufferIn[c];
        delete[] mConversionBufferOut[c];
      }
      delete[] mConversionBufferIn;
      delete[] mConversionBufferOut;
      mConversionBufferIn = nullptr;
      mConversionBufferOut = nullptr;
#endif
    }

    void resampleAndLoadIR(float** samples, size_t sampleCount, size_t sampleRate, size_t channelCount) {
      mIRLoaded = false;
      for (int c = 0; c < channelCount; c++) {
        WindowedSincResampler<float, float> resampler(sampleRate, mSampleRate);
        float* outBuffer = nullptr;
        const size_t outSamples = resampler.resample(samples[c], sampleCount, &outBuffer, (sampleRate / static_cast<float>(mSampleRate)) * 0.2f);
        if (channelCount == 1) {
          for (int ch = 0; ch < CHANNEL_COUNT; ch++) {
            mConvolvers[ch]->init(CONV_BLOCK_SIZE, CONV_TAIL_BLOCK_SIZE, outBuffer, outSamples);
          }
        }
        else if (channelCount == CHANNEL_COUNT) {
          mConvolvers[c]->init(CONV_BLOCK_SIZE, CONV_TAIL_BLOCK_SIZE, outBuffer, outSamples);
        }
        delete[] outBuffer;
      }
      mIRLoaded = true;
    }

    void ProcessBlock(sample** in, sample** out, const int nFrames) {
      if (!mIRLoaded) {
        for (int c = 0; c < CHANNEL_COUNT; c++) {
          for (int i = 0; i < nFrames; i++) {
            out[c][i] = in[c][i];
          }
        }
        return;
      }
#ifdef GUITARD_FLOAT_CONVOLUTION
      /**                           THREADPOOLING ATTEMPT                           */
#ifdef useThreadPool
      std::future<void> right;
      if (mStereo) {
        right = tPool.push([&](int id) {
          for (int i = 0; i < nFrames; i++) {
            mConversionBufferIn[1][i] = in[1][i];
          }
          mConvolvers[1]->process(mConversionBufferIn[1], mConversionBufferOut[1], nFrames);
          for (int i = 0; i < nFrames; i++) {
            out[1][i] = mConversionBufferOut[1][i];
          }
        });
      }

      for (int i = 0; i < nFrames; i++) {
        mConversionBufferIn[0][i] = in[0][i];
      }
      mConvolvers[0]->process(mConversionBufferIn[0], mConversionBufferOut[0], nFrames);
      for (int i = 0; i < nFrames; i++) {
        out[0][i] = mConversionBufferOut[0][i];
      }

      if (!mStereo) {
        for (int i = 0; i < nFrames; i++) {
          out[1][i] = out[0][i];
        }
      }
      else {
        right.wait();
      }

#else
    /**                           OPENMP ATTEMPT                           */
#ifdef useOpenMP
      if (mStereo) {
#pragma omp parallel num_threads(2)
        {
          int id = omp_get_thread_num();
          for (int i = 0; i < nFrames; i++) {
            mConversionBufferIn[id][i] = in[id][i];
          }
          mConvolvers[id]->process(mConversionBufferIn[id], mConversionBufferOut[id], nFrames);
          for (int i = 0; i < nFrames; i++) {
            out[id][i] = mConversionBufferOut[id][i];
          }
        }
      }
#else
    /**                           REFERENCE                           */
      for (int i = 0; i < nFrames; i++) {
        mConversionBufferIn[0][i] = static_cast<float>(in[0][i]);
      }
      mConvolvers[0]->process(mConversionBufferIn[0], mConversionBufferOut[0], nFrames);
      for (int i = 0; i < nFrames; i++) {
        out[0][i] = mConversionBufferOut[0][i];
      }

      if (!mStereo) {
        for (int i = 0; i < nFrames; i++) {
          out[1][i] = out[0][i];
        }
      }
      else {
        for (int i = 0; i < nFrames; i++) {
          mConversionBufferIn[1][i] = static_cast<float>(in[1][i]);
        }
        mConvolvers[1]->process(mConversionBufferIn[1], mConversionBufferOut[1], nFrames);
        for (int i = 0; i < nFrames; i++) {
          out[1][i] = mConversionBufferOut[1][i];
        }
      }
#endif

#endif

#else
      convolver.process(buffer[0], outputs[0][0], nFrames);
      if (mStereo) {
        convolver2.process(buffer[1], outputs[0][1], nFrames);
      }
      else {
        for (int i = 0; i < nFrames; i++) {
          outputs[0][1][i] = outputs[0][0][i];
        }
      }
#endif
    }

    static std::string getLicense() {
      std::string l = "Realtime Convolution by\n";
      l += "https://github.com/HiFi-LoFi\n";
      l += "https://github.com/HiFi-LoFi/FFTConvolver\n";
      l += "MIT License\n\n";
      l += "Wave file reader \"dr_wav\"";
      l += "https://github.com/mackron\n";
      l += "https://github.com/mackron/dr_libs/blob/master/dr_wav.h\n";
      l += "Public domain";
      return l;
    }
  };
}