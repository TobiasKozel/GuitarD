#pragma once

#include "../GConfig.h"
#include "./GTypes.h"

#include <functional>

#ifdef GUITARD_SSE
  #ifdef SAMPLE_TYPE_FLOAT
    #include "../../thirdparty/HIIR/Downsampler2xSse.h"
    #include "../../thirdparty/HIIR/Upsampler2xSse.h"
  #else
    #include "../../thirdparty/HIIR/Upsampler2xF64Sse2.h"
    #include "../../thirdparty/HIIR/Downsampler2xF64Sse2.h"
  #endif
  
#else
  #include "../../thirdparty/HIIR/Upsampler2xFpu.h"
  #include "../../thirdparty/HIIR/Downsampler2xFpu.h"
#endif


namespace guitard {
  /**
   * Super simple wrapper for the hiir up and down samplers
   * It only goes up to 4x oversampling 
   */
  class Oversampler {
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

    // Stack buffers are more convenient
    T mBufs2x[mChannels * 2][GUITARD_MAX_BUFFER * 2];
    T* mBuf2xUp[mChannels] = { mBufs2x[0], mBufs2x[1] };
    T* mBuf2xDown[mChannels] = { mBufs2x[2], mBufs2x[3] };

    T mBufs4x[mChannels * 2][GUITARD_MAX_BUFFER * 4];
    T* mBuf4xUp[mChannels] = { mBufs4x[0], mBufs4x[1] };
    T* mBuf4xDown[mChannels] = { mBufs4x[2], mBufs4x[3] };

    /**
     * Straight up stolen from the hiir oversampler wrapper from iPlug2
     * https://github.com/iPlug2/iPlug2/blob/master/IPlug/Extras/Oversampler.h
     */
    const double coeffs2x[12] = { 0.036681502163648017, 0.13654762463195794, 0.27463175937945444, 0.42313861743656711, 0.56109869787919531, 0.67754004997416184, 0.76974183386322703, 0.83988962484963892, 0.89226081800387902, 0.9315419599631839, 0.96209454837808417, 0.98781637073289585 };
    const double coeffs4x[4] = { 0.041893991997656171, 0.16890348243995201, 0.39056077292116603, 0.74389574826847926 };
  public:
    unsigned int mFactor = 1;
    using ProcessFunction = std::function<void(T**, T**, int)>;
    ProcessFunction mProc;

    Oversampler() {
      for (int c = 0; c < mChannels; c++) {
        mUp2x[c].set_coefs(coeffs2x);
        mDown2x[c].set_coefs(coeffs2x);
        mUp4x[c].set_coefs(coeffs4x);
        mDown4x[c].set_coefs(coeffs4x);
      }
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
      if (mFactor == 2 || mFactor == 3) {
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