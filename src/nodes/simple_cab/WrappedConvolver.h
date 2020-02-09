#pragma once
// #define useThreadPool
// #define useOpenMP

#ifdef useOpenMP
  #include <omp.h>
#endif

#include "src/misc/constants.h"
#include "src/types/types.h"
#ifndef GUITARD_FLOAT_CONVOLUTION
  #define FFTCONVOLVER_TYPE guitard::sample
#endif

#include "convolver/twoStageConvolver.h"
#include "threadpool.h"
#include "src/types/resampler.h"

namespace guitard {
  class WrappedConvolver {
    const int CONV_BLOCK_SIZE = 128;
    const int CONV_TAIL_BLOCK_SIZE = 1024 * 4;
    const int CHANNEL_COUNT = 2;
    int mMaxBuffer = 0;
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

    // IRBundle* mLoadedIr = nullptr;
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
#ifdef GUITARD_FLOAT_CONVOLUTION
      mConversionBufferIn = new WDL_RESAMPLE_TYPE * [CHANNEL_COUNT];
      mConversionBufferOut = new WDL_RESAMPLE_TYPE * [CHANNEL_COUNT];
      for (int c = 0; c < CHANNEL_COUNT; c++) {
        mConversionBufferIn[c] = new WDL_RESAMPLE_TYPE[mMaxBuffer];
        mConversionBufferOut[c] = new WDL_RESAMPLE_TYPE[mMaxBuffer];
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

    void resampleAndLoadIR(IRBundle* b) {
      mIRLoaded = false;
      unloadWave(b);
      if (!loadWave(b)) {
        WDBGMSG("Failed to load IR!\n");
        return;
      }
      for (int c = 0; c < b->channelCount; c++) {
        LinearResampler<float, float> resampler(b->sampleRate, mSampleRate);
        float* outBuffer = nullptr;
        const size_t outSamples = resampler.resample(b->samples[c], b->sampleCount, &outBuffer, (b->sampleRate / static_cast<float>(mSampleRate)) * 0.2f);
        if (b->channelCount == 1) {
          for (int ch = 0; ch < CHANNEL_COUNT; ch++) {
            mConvolvers[ch]->init(CONV_BLOCK_SIZE, CONV_TAIL_BLOCK_SIZE, outBuffer, outSamples);
          }
        }
        else if (b->channelCount == CHANNEL_COUNT) {
          mConvolvers[c]->init(CONV_BLOCK_SIZE, CONV_TAIL_BLOCK_SIZE, outBuffer, outSamples);
        }
        delete[] outBuffer;
      }
      mIRLoaded = true;
    }

    /**
     * Only takes a pointer to the ir object to load since it won't really be needed for dsp
     * It will put the loaded wave in the samples buffer of the ir bundle provided
     * Call unloadWave to get rid of the allocated buffer
     */
    static bool loadWave(IRBundle* b) {
      if (b->samples != nullptr) { return true; } // Already loaded samples
      if (b->path.isEmpty()) { return false; }
      drwav wav;
      // load the file
      if (!drwav_init_file(&wav, b->path.get(), nullptr)) {
        return false;
      }

      // get space for the samples
      float* pSampleData = static_cast<float*>(malloc(
        static_cast<size_t>(wav.totalPCMFrameCount)* wav.channels * sizeof(float)
      ));
      const size_t samplesRead = drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, pSampleData);
      const float length = samplesRead / static_cast<float>(wav.sampleRate);
      if (length > 10.f || samplesRead < 1) {
        // don't load anything longer than 10 seconds or without samples
        assert(false);
      }
      b->sampleRate = wav.sampleRate;
      b->channelCount = wav.channels;
      b->sampleCount = samplesRead;
      b->samples = new float* [b->channelCount];
      // do some deinterleaving 
      for (int c = 0; c < b->channelCount; c++) {
        b->samples[c] = new float[b->sampleCount];
      }
      for (int s = 0; s < samplesRead * b->channelCount; s++) {
        int channel = s % b->channelCount;
        size_t sample = s / b->channelCount;
        b->samples[channel][sample] = pSampleData[s];
      }
      free(pSampleData);
      drwav_uninit(&wav);
      return true;
    }

    /**
     * Will clear the buffer allocated in loadWave
     */
    static void unloadWave(IRBundle* b) {
      if (b->path.getLength() > 0 && b->samples != nullptr) {
        // Check if the previous IR is custom and clear out the allocated data
        for (int c = 0; c < b->channelCount; c++) {
          delete[] b->samples[c];
        }
        delete[] b->samples;
        b->samples = nullptr;
      }
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