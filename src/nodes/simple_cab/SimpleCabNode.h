#pragma once
#include "thirdparty/fftconvolver/TwoStageFFTConvolver.h"
#include "resample.h"
#include "src/node/Node.h"
#include "thirdparty/threadpool.h"
#include "filebrowse.h"
#include "InternalIRs.h"

#define DR_WAV_IMPLEMENTATION
#include "thirdparty/dr_wav.h"

// TODOG figure out this swell stuff
#ifdef FillRect
#undef FillRect
#endif
#ifdef DrawText
#undef DrawText
#endif


// #define useThreadPool
// #define useOpenMP

#ifdef useOpenMP
#include <omp.h>
#endif


class SimpleCabNodeUi : public NodeUi {
  iplug::igraphics::IText mBlocksizeText;
  IVButtonControl* mBrowseButton = nullptr;
  string mInfo;
public:
  SimpleCabNodeUi(NodeShared* param) : NodeUi(param) {
    mInfo = "None";
    mBlocksizeText = DEBUG_FONT;
  }

  void setUpControls() override {
    NodeUi::setUpControls();
    const IRECT button{ mTargetRECT.L + 50, mTargetRECT.T + 100, mTargetRECT.R - 50, mTargetRECT.B - 20 };
    mBrowseButton = new IVButtonControl(button, [&](IControl* pCaller) {
      this->openFileDialog();
    });
    mElements.Add(mBrowseButton);
    shared->graphics->AttachControl(mBrowseButton);
  }


  void openFileDialog() const {
    HWND handle = reinterpret_cast<HWND>(shared->graphics->GetWindow());
    const char* result = WDL_ChooseFileForOpen(
      handle, "Open IR", nullptr, nullptr, "Wave Files\0*.wav;*.WAV\0AIFF Files\0*.aiff;*.AIFF\0", "*.wav",
      true, false
    );
    if (result != nullptr) {
      WDBGMSG(result);
      // loadWave(result);
    }
    else {
      WDBGMSG("No file selected.\n");
    }
    // This is needed so the button which opened the pop up doesn't trigger the dialog again
    shared->graphics->ReleaseMouseCapture();
  }

  /**
   * File drop is only supported in the standalone app
   */
  void OnDrop(const char* str) override {
    mInfo = "Path: " + string(str);
    mDirty = true;
  }

  void Draw(IGraphics& g) override {
    NodeUi::Draw(g);
    g.DrawText(mBlocksizeText, mInfo.c_str(), mRECT);
  }

  void cleanUp() const override {
    NodeUi::cleanUp();
    shared->graphics->RemoveControl(mBrowseButton, true);
  }
};

class SimpleCabNode final : public Node {
  fftconvolver::TwoStageFFTConvolver* mConvolvers[8] = { nullptr };
  WDL_RESAMPLE_TYPE** mConversionBufferIn = nullptr;
  WDL_RESAMPLE_TYPE** mConversionBufferOut = nullptr;
  bool mIRLoaded = false;
  IRBundle mLoadedIR;

#ifdef useThreadPool
  ctpl::thread_pool tPool;
#endif
  
public:
  SimpleCabNode(std::string pType) : Node() {
    mType = pType;
  }

  void setup(MessageBus::Bus* pBus, int pSamplerate = 48000, int pMaxBuffer = MAX_BUFFER, int pChannles = 2, int pInputs = 1, int pOutputs = 1) override {
    Node::setup(pBus, pSamplerate, pMaxBuffer, 2, 1, 1);

    mStereo = 0;
    addByPassParam();
    addStereoParam();
#ifdef useThreadPool
    tPool.resize(2);
#endif
  }

  void resampleAndLoadIR(const IRBundle b) {
    mIRLoaded = false;
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
        for (int ch = 0; ch < mChannelCount; ch++) {
          mConvolvers[ch]->init(128, 1024 * 4, outBuffer, outSamples);
        }
      }
      else if (b.channelCount == mChannelCount) {
        mConvolvers[c]->init(128, 1024 * 4, outBuffer, outSamples);
      }
      delete[] outBuffer;
    }
    mLoadedIR = b;
    mIRLoaded = true;
  }

  static void loadWave(const char* path, bool embed = false) {
    drwav wav;
    if (!drwav_init_file(&wav, path, nullptr)) {
      return;
    }

    float* pSampleData = static_cast<float*>(malloc(
      static_cast<size_t>(wav.totalPCMFrameCount) * wav.channels * sizeof(float)
    ));
    const size_t samplesRead = drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, pSampleData);
    const float length = samplesRead / static_cast<float>(wav.sampleRate);
    if (length > 10.f || samplesRead < 1) {
      assert(false);
    }
    drwav_uninit(&wav);
  }


  void createBuffers() override {
    Node::createBuffers();
    for (int c = 0; c < mChannelCount; c++) {
      mConvolvers[c] = new fftconvolver::TwoStageFFTConvolver();
    }

#ifdef FLOATCONV
    mConversionBufferIn = new WDL_RESAMPLE_TYPE * [mChannelCount];
    mConversionBufferOut = new WDL_RESAMPLE_TYPE * [mChannelCount];
    for (int c = 0; c < mChannelCount; c++) {
      mConversionBufferIn[c] = new WDL_RESAMPLE_TYPE[mMaxBuffer];
      mConversionBufferOut[c] = new WDL_RESAMPLE_TYPE[mMaxBuffer];
    }
#endif
    resampleAndLoadIR(InternalIRs[1]);
  }

  void deleteBuffers() override {
    Node::deleteBuffers();
   
    for (int c = 0; c < mChannelCount; c++) {
      delete mConvolvers[c];
    }
#ifdef FLOATCONV
    for (int c = 0; c < mChannelCount; c++) {
      delete[] mConversionBufferIn[c];
      delete[] mConversionBufferOut[c];
    }
    delete[] mConversionBufferIn;
    delete[] mConversionBufferOut;
    mConversionBufferIn = nullptr;
    mConversionBufferOut = nullptr;
#endif
  }

  void ProcessBlock(const int nFrames) {
    if (!inputsReady() || mIsProcessed || byPass()) { return; }
    if (!mIRLoaded) {
      outputSilence();
      return;
    }
    shared.parameters.Get(1)->update();

    sample** buffer = shared.socketsIn.Get(0)->mConnectedTo->mParentBuffer;

#ifdef FLOATCONV
    /**                           THREADPOOLING ATTEMPT                           */
#ifdef useThreadPool
    std::future<void> right;
    if (mStereo) {
      right = tPool.push([&](int id) {
        for (int i = 0; i < nFrames; i++) {
          conversionBufferIn[1][i] = buffer[1][i];
        }
        convolvers[1]->process(conversionBufferIn[1], conversionBufferOut[1], nFrames);
        for (int i = 0; i < nFrames; i++) {
          outputs[0][1][i] = conversionBufferOut[1][i];
        }
      });
    }

    for (int i = 0; i < nFrames; i++) {
      conversionBufferIn[0][i] = buffer[0][i];
    }
    convolvers[0]->process(conversionBufferIn[0], conversionBufferOut[0], nFrames);
    for (int i = 0; i < nFrames; i++) {
      outputs[0][0][i] = conversionBufferOut[0][i];
    }

    if(!mStereo) {
      for (int i = 0; i < nFrames; i++) {
        outputs[0][1][i] = outputs[0][0][i];
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
          conversionBufferIn[id][i] = buffer[id][i];
        }
        convolvers[id]->process(conversionBufferIn[id], conversionBufferOut[id], nFrames);
        for (int i = 0; i < nFrames; i++) {
          outputs[0][id][i] = conversionBufferOut[id][i];
        }
      }
    }
#else
    /**                           REFERENCE                           */
    for (int i = 0; i < nFrames; i++) {
      mConversionBufferIn[0][i] = static_cast<float>(buffer[0][i]);
    }
    mConvolvers[0]->process(mConversionBufferIn[0], mConversionBufferOut[0], nFrames);
    for (int i = 0; i < nFrames; i++) {
      mBuffersOut[0][0][i] = mConversionBufferOut[0][i];
    }

    if (!mStereo) {
      for (int i = 0; i < nFrames; i++) {
        mBuffersOut[0][1][i] = mBuffersOut[0][0][i];
      }
    }
    else {
      for (int i = 0; i < nFrames; i++) {
        mConversionBufferIn[1][i] = static_cast<float>(buffer[1][i]);
      }
      mConvolvers[1]->process(mConversionBufferIn[1], mConversionBufferOut[1], nFrames);
      for (int i = 0; i < nFrames; i++) {
        mBuffersOut[0][1][i] = mConversionBufferOut[1][i];
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
    mIsProcessed = true;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    shared.graphics = pGrahics;
    mUi = new SimpleCabNodeUi(&shared);
    pGrahics->AttachControl(mUi);
    mUi->setColor(IColor(255, 150, 100, 100));
    mUi->setUp();
    mUiReady = true;
    
  }
};
