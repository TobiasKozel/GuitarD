#pragma once
// #define useThreadPool
// #define useOpenMP

#ifdef useOpenMP
#include <omp.h>
#endif
#include "thirdparty/fftconvolver/TwoStageFFTConvolver.h"
#include "resample.h"
#include "thirdparty/threadpool.h"
#include "InternalIRs.h"
#define DR_WAV_IMPLEMENTATION
#include "thirdparty/dr_wav.h"
#include "IPlugConstants.h"

class WrappedConvolver {
  const int CONV_BLOCK_SIZE = 128;
  const int CONV_TAIL_BLOCK_SIZE = 1024 * 4;
  const int CHANNEL_COUNT = 2;
  int mMaxBuffer = 0;
  int mLastBlockSize = 0;
#ifdef useThreadPool
  ctpl::thread_pool tPool;
#endif

  /** We'll only do stereo convolution at most */
  fftconvolver::TwoStageFFTConvolver* mConvolvers[2] = { nullptr };

  /** Buffers need to be converted from double to float */
  WDL_RESAMPLE_TYPE** mConversionBufferIn = nullptr;
  WDL_RESAMPLE_TYPE** mConversionBufferOut = nullptr;

  int mSampleRate = 0;

  bool mIRLoaded = false;

  IRBundle mLoadedIr;

  bool mWarmUpLeft = CONV_BLOCK_SIZE;
public:

  bool mStereo = false;

  explicit WrappedConvolver(const int samplerate = 48000, const int maxbuffer = 512) {
#ifdef useThreadPool
    tPool.resize(2);
#endif
    mSampleRate = samplerate;
    for (int c = 0; c < CHANNEL_COUNT; c++) {
      mConvolvers[c] = new fftconvolver::TwoStageFFTConvolver();
    }
    mMaxBuffer = maxbuffer;
#ifdef FLOATCONV
    mConversionBufferIn = new WDL_RESAMPLE_TYPE * [CHANNEL_COUNT];
    mConversionBufferOut = new WDL_RESAMPLE_TYPE * [CHANNEL_COUNT];
    for (int c = 0; c < CHANNEL_COUNT; c++) {
      mConversionBufferIn[c] = new WDL_RESAMPLE_TYPE[mMaxBuffer];
      mConversionBufferOut[c] = new WDL_RESAMPLE_TYPE[mMaxBuffer];
    }
#endif
  }

  ~WrappedConvolver() {
    clearLoadedIR();
    for (int c = 0; c < CHANNEL_COUNT; c++) {
      delete mConvolvers[c];
    }
#ifdef FLOATCONV
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

  void clearLoadedIR() {
    if (mLoadedIr.path.GetLength() > 0 && mLoadedIr.samples != nullptr) {
      // Check if the previous IR is custom and clear out the allocated data
      for (int c = 0; c < mLoadedIr.channelCount; c++) {
        delete[] mLoadedIr.samples[c];
      }
      delete[] mLoadedIr.samples;
      mLoadedIr.samples = nullptr;
    }
  }

  void resampleAndLoadIR(IRBundle& b) {
    mIRLoaded = false;
    clearLoadedIR();
    if (b.path.GetLength() > 0 && !loadWave(b)) {
      WDBGMSG("Failed to load IR!\n");
      return;
    }
    for (int c = 0; c < b.channelCount; c++) {
      WDL_Resampler resampler;
      resampler.SetMode(true, 0, true);
      resampler.SetFilterParms();
      resampler.SetFeedMode(true);
      resampler.SetRates(b.sampleRate, mSampleRate);
      WDL_RESAMPLE_TYPE* inBuffer;
      const int inSamples = resampler.ResamplePrepare(b.sampleCount, 1, &inBuffer);
      for (int i = 0; i < b.sampleCount; i++) {
        // Adjust the volume, since higher samplerates result in louder outputs
        inBuffer[i] = b.samples[c][i] * (b.sampleRate / static_cast<float>(mSampleRate)) * 0.2f;
      }
      const int newSize = ceil(b.sampleCount * ((mSampleRate / static_cast<float>(b.sampleRate))));
      WDL_RESAMPLE_TYPE* outBuffer = new WDL_RESAMPLE_TYPE[newSize];
      const int outSamples = resampler.ResampleOut(outBuffer, inSamples, b.sampleCount, 1);
      if (b.channelCount == 1) {
        for (int ch = 0; ch < CHANNEL_COUNT; ch++) {
          mConvolvers[ch]->init(CONV_BLOCK_SIZE, CONV_TAIL_BLOCK_SIZE, outBuffer, outSamples);
        }
      }
      else if (b.channelCount == CHANNEL_COUNT) {
        mConvolvers[c]->init(CONV_BLOCK_SIZE, CONV_TAIL_BLOCK_SIZE, outBuffer, outSamples);
      }
      delete[] outBuffer;
    }
    mLoadedIr = b;
    mWarmUpLeft = true;
    mIRLoaded = true;
  }

  static bool loadWave(IRBundle& b) {
    if (b.samples != nullptr) { return true; } // Already loaded samples
    drwav wav;
    if (!drwav_init_file(&wav, b.path.Get(), nullptr)) {
      return false;
    }

    float* pSampleData = static_cast<float*>(malloc(
      static_cast<size_t>(wav.totalPCMFrameCount)* wav.channels * sizeof(float)
    ));
    const size_t samplesRead = drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, pSampleData);
    const float length = samplesRead / static_cast<float>(wav.sampleRate);
    if (length > 10.f || samplesRead < 1) {
      assert(false);
    }
    b.sampleRate = wav.sampleRate;
    b.channelCount = wav.channels;
    b.sampleCount = samplesRead;
    b.samples = new float* [b.channelCount];
    for (int c = 0; c < b.channelCount; c++) {
      b.samples[c] = new float[b.sampleCount];
    }
    for (int s = 0; s < samplesRead * b.channelCount; s++) {
      int channel = s % b.channelCount;
      size_t sample = s / b.channelCount;
      b.samples[channel][sample] = pSampleData[s];
    }
    free(pSampleData);
    drwav_uninit(&wav);
    return true;
  }

  void ProcessBlock(iplug::sample** in, iplug::sample** out, const int nFrames) {
    if (!mIRLoaded) {
      for (int c = 0; c < CHANNEL_COUNT; c++) {
        for (int i = 0; i < nFrames; i++) {
          out[c][i] = in[c][i];
        }
      }
      return;
    }
#ifdef FLOATCONV
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
    if (mWarmUpLeft) {
      mWarmUpLeft = false;
      iplug::sample scaling = 1.f / static_cast<iplug::sample>(nFrames);
      for (int c = 0; c < CHANNEL_COUNT; c++) {
        iplug::sample last = out[c][mLastBlockSize - 1];
        for (int i = 0; i < nFrames; i++) {
          const iplug::sample s = scaling * i;
          out[c][i] = in[c][i] * s + last * (1 - s);
        }
      }
    }
    mLastBlockSize = nFrames;
  }
};