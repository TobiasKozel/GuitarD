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
  /**
   * This is a stripped down version of the HIIR oversampler from iPlug2
   * https://github.com/iPlug2/iPlug2/blob/master/IPlug/Extras/Oversampler.h
   * It only goes up to 4x oversampling 
   */
  class HiirOverSampler {
    static constexpr int mChannels = 2;
    using T = sample;

#ifdef GUITARD_SSE
  #ifdef SAMPLE_TYPE_FLOAT
    hiir::Upsampler2xSse<12> mUp2x[mChannels];
    hiir::Downsampler2xSse<12> mDown2x[mChannels];
    hiir::Upsampler2xSse<4> mUp4x[mChannels];
    hiir::Downsampler2xSse<4> mDown4x[mChannels];
  #else
    hiir::Upsampler2xF64Sse2<12> mUp2x[mChannels];
    hiir::Downsampler2xF64Sse2<12> mDown2x[mChannels];
    hiir::Upsampler2xF64Sse2<4> mUp4x[mChannels];
    hiir::Downsampler2xF64Sse2<4> mDown4x[mChannels];
  #endif
#else
    hiir::Upsampler2xFpuTpl<12, T> mUp2x[mChannels];
    hiir::Downsampler2xFpuTpl<12, T> mDown2x[mChannels];
    hiir::Upsampler2xFpuTpl<4, T> mUp4x[mChannels];
    hiir::Downsampler2xFpuTpl<4, T> mDown4x[mChannels];
#endif

    T** mBuf2xUp = nullptr;
    T** mBuf2xDown = nullptr;
    T** mBuf4xUp = nullptr;
    T** mBuf4xDown = nullptr;
    static constexpr double coeffs2x[12] = { 0.036681502163648017, 0.13654762463195794, 0.27463175937945444, 0.42313861743656711, 0.56109869787919531, 0.67754004997416184, 0.76974183386322703, 0.83988962484963892, 0.89226081800387902, 0.9315419599631839, 0.96209454837808417, 0.98781637073289585 };
    static constexpr double coeffs4x[4] = { 0.041893991997656171, 0.16890348243995201, 0.39056077292116603, 0.74389574826847926 };
  public:
    int mFactor = 1;
    using ProcessFunction = std::function<void(T**, T**, int)>;
    ProcessFunction mProc;

  

    HiirOverSampler() {
      mBuf2xUp   = new T* [mChannels];
      mBuf2xDown = new T* [mChannels];
      mBuf4xUp   = new T* [mChannels];
      mBuf4xDown = new T* [mChannels];

      for (int c = 0; c < mChannels; c++) {
        mUp2x[c].set_coefs(coeffs2x);
        mDown2x[c].set_coefs(coeffs2x);
        mUp4x[c].set_coefs(coeffs4x);
        mDown4x[c].set_coefs(coeffs4x);
        mBuf2xDown[c] = new T[GUITARD_MAX_BUFFER * 2];
        mBuf2xUp[c]   = new T[GUITARD_MAX_BUFFER * 2];
        mBuf4xDown[c] = new T[GUITARD_MAX_BUFFER * 4];
        mBuf4xUp[c]   = new T[GUITARD_MAX_BUFFER * 4];
      }
    }

    ~HiirOverSampler() {
      for (int c = 0; c < mChannels; c++) {
        delete[] mBuf2xDown[c];
        delete[] mBuf2xUp[c];
        delete[] mBuf4xDown[c];
        delete[] mBuf4xUp[c];
      }
      delete[] mBuf2xDown;
      delete[] mBuf2xUp;
      delete[] mBuf4xDown;
      delete[] mBuf4xUp;
    }

    void setFactor(const int factor) {
      mFactor = factor;
    }

    void process(T** in, T** out, const int frames) {
      /**
       * No OverSampling at all
       */
      if (mFactor == 1) {
        mProc(in, out, frames);
        return;
      }

      /**
       * 2x OverSampling
       */
      if (mFactor == 2) {
        for (int c = 0; c < mChannels; c++) {
          mUp2x[c].process_block(mBuf2xUp[c], in[c], frames);
        }

        mProc(mBuf2xUp, mBuf2xDown, frames * 2);

        for (int c = 0; c < mChannels; c++) {
          mDown2x[c].process_block(out[c], mBuf2xDown[c], frames);
        }
        return;
      }

      /**
       * 4x OverSampling
       */
      for (int c = 0; c < mChannels; c++) {
        mUp2x[c].process_block(mBuf2xUp[c], in[c], frames);
        mUp4x[c].process_block(mBuf4xUp[c], mBuf2xUp[c], frames * 2);
      }

      mProc(mBuf4xUp, mBuf4xDown, frames * 4);

      for (int c = 0; c < mChannels; c++) {
        mDown4x[c].process_block(mBuf2xDown[c], mBuf4xDown[c], frames * 2);
        mDown2x[c].process_block(out[c], mBuf2xDown[c], frames);
      }
    }
  };
}