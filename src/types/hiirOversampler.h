#pragma once
#include "types.h"
#include <functional>

#ifdef GUITARD_SSE
  #ifdef SAMPLE_TYPE_FLOAT
    #include "../../thirdparty/hiir/Downsampler2xSse.h"
    #include "../../thirdparty/hiir/Upsampler2xSse.h"
  #else
    #include "../../thirdparty/hiir/Upsampler2xF64Sse2.h"
    #include "../../thirdparty/hiir/Downsampler2xF64Sse2.h"
  #endif
  
#else
  #include "../../thirdparty/hiir/Upsampler2xFpu.h"
  #include "../../thirdparty/hiir/Downsampler2xFpu.h"
#endif


namespace guitard {
  class HiirOverSampler {
    static constexpr int mChannels = 2;
    using T = sample;

#ifdef GUITARD_SSE
  #ifdef SAMPLE_TYPE_FLOAT
    hiir::Upsampler2xSse<12> mUp2x[mChannels];
    hiir::Downsampler2xSse<12> mDown2x[mChannels];
  #else
    hiir::Upsampler2xF64Sse2<12> mUp2x[mChannels];
    hiir::Downsampler2xF64Sse2<12> mDown2x[mChannels];
  #endif
#else
    hiir::Upsampler2xFpuTpl<12, T> mUp2x[mChannels];
    hiir::Downsampler2xFpuTpl<12, T> mDown2x[mChannels];
#endif

    sample** mBuf2xUp = nullptr;
    sample** mBuf2xDown = nullptr;
    static constexpr double coeffs2x[12] = { 0.036681502163648017, 0.13654762463195794, 0.27463175937945444, 0.42313861743656711, 0.56109869787919531, 0.67754004997416184, 0.76974183386322703, 0.83988962484963892, 0.89226081800387902, 0.9315419599631839, 0.96209454837808417, 0.98781637073289585 };
  public:
    int mFactor = 1;
    using ProcessFunction = std::function<void(T**, T**, int)>;
    ProcessFunction mProc;

  

    HiirOverSampler() {
      mBuf2xUp   = new sample* [mChannels];
      mBuf2xDown = new sample* [mChannels];
      for (int c = 0; c < mChannels; c++) {
        mUp2x[c].set_coefs(coeffs2x);
        mDown2x[c].set_coefs(coeffs2x);
        mBuf2xDown[c] = new sample[GUITARD_MAX_BUFFER * 2];
        mBuf2xUp[c]   = new sample[GUITARD_MAX_BUFFER * 2];
      }
    }

    ~HiirOverSampler() {
      for (int c = 0; c < mChannels; c++) {
        delete[] mBuf2xDown[c];
        delete[] mBuf2xUp[c];
      }
      delete[] mBuf2xDown;
      delete[] mBuf2xUp;
    }

    void setFactor(const int factor) {
      mFactor = factor;
    }

    void process(T** in, T** out, const int frames) {
      mFactor = std::min(mFactor, 2);
      if (mFactor == 1) {
        mProc(in, out, frames);
        return;
      }
      for (int c = 0; c < mChannels; c++) {
        mUp2x[c].process_block(mBuf2xUp[c], in[c], frames);
      }

      mProc(mBuf2xUp, mBuf2xDown, frames * mFactor);

      for (int c = 0; c < mChannels; c++) {
        mDown2x[c].process_block(out[c], mBuf2xDown[c], frames);
      }
    }
  };
}