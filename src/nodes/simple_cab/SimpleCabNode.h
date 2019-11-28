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

struct CabNodeSharedData {
  function<void(IRBundle)> callback;
  IRBundle loadedIr;
  bool embedIr = false;
};

class SimpleCabNodeUi : public NodeUi {
  iplug::igraphics::IText mBlocksizeText;
  IVButtonControl* mBrowseButton = nullptr;
  CabNodeSharedData* mCabShared = nullptr;
  IPopupMenu mMenu{ "Choose IR", {"Clean", "Air", "From File"}, [&](int indexInMenu, IPopupMenu::Item* itemChosen) {
    if (itemChosen) {
      const char* text = itemChosen->GetText();
      if (strncmp(text, "From File", 10) == 0) {
        this->openFileDialog();
      }
      else {
        for (int i = 0; i < InternalIRsCount; i++) {
          if (strncmp(InternalIRs[i].name.Get(), text, 30) == 0) {
            mCabShared->callback(InternalIRs[i]);
          }
        }
      }
    }
  }};

public:
  SimpleCabNodeUi(NodeShared* param) : NodeUi(param) {
    mBlocksizeText = DEBUG_FONT;
  }

  void setUpControls() override {
    NodeUi::setUpControls();
    const IRECT button{ mTargetRECT.L + 50, mTargetRECT.T + 130, mTargetRECT.R - 50, mTargetRECT.B - 20 };
    mBrowseButton = new IVButtonControl(button, [&](IControl* pCaller) {
      SplashClickActionFunc(pCaller);
      float x, y;
      this->shared->graphics->GetMouseDownPoint(x, y);
      this->shared->graphics->CreatePopupMenu(*pCaller, mMenu, x, y);

    }, "Select IR", DEFAULT_STYLE, true, false);
    mElements.Add(mBrowseButton);
    shared->graphics->AttachControl(mBrowseButton);
  }


  void openFileDialog() const {
    const HWND handle = reinterpret_cast<HWND>(shared->graphics->GetWindow());
    char* result = WDL_ChooseFileForOpen(
      handle, "Open IR", nullptr, nullptr,
      //"Wave Files\0*.wav;*.WAV\0AIFF Files\0*.aiff;*.AIFF\0", "*.wav",
      "Wave Files\0*.wav;*.WAV\0", "*.wav",
      true, false
    );
    if (result != nullptr) {
      WDBGMSG(result);
      IRBundle load;
      load.path.Set(result);
      load.name.Set(load.path.get_filepart());
      free(result);
      mCabShared->callback(load);
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
    IRBundle load;
    load.path.Set(str);
    load.name.Set(load.path.get_filepart());
    if (strncmp(load.name.get_fileext(), ".wav", 4) == 0) {
      mCabShared->callback(load);
    }
  }

  void Draw(IGraphics& g) override {
    NodeUi::Draw(g);
    if (mCabShared != nullptr) {
      g.DrawText(mBlocksizeText, mCabShared->loadedIr.name.Get(), mRECT.GetVShifted(20));
    }
  }

  void cleanUp() override {
    NodeUi::cleanUp();
    shared->graphics->RemoveControl(mBrowseButton, true);
  }

  void registerSharedData(CabNodeSharedData* data) {
    mCabShared = data;
  }
};


class SimpleCabNode final : public Node {
  fftconvolver::TwoStageFFTConvolver* mConvolvers[8] = { nullptr };
  WDL_RESAMPLE_TYPE** mConversionBufferIn = nullptr;
  WDL_RESAMPLE_TYPE** mConversionBufferOut = nullptr;
  bool mIRLoaded = false;
  CabNodeSharedData mCabShared = { [&](IRBundle ir) {
    this->resampleAndLoadIR(ir);
  }, IRBundle(), false };

#ifdef useThreadPool
  ctpl::thread_pool tPool;
#endif
  
public:
  SimpleCabNode(const std::string pType) {
    mType = pType;
    mStereo = 0;
    addByPassParam();
    addStereoParam();
    shared.parameters[1]->y = -30;
#ifdef useThreadPool
    tPool.resize(2);
#endif
  }

  void clearLoadedIR() const {
    if (mCabShared.loadedIr.path.GetLength() != 0 && mCabShared.loadedIr.samples != nullptr) {
      // Check if the previous IR is custom and clear out the allocated data
      for (int c = 0; c < mCabShared.loadedIr.channelCount; c++) {
        delete[] mCabShared.loadedIr.samples[c];
      }
      delete[] mCabShared.loadedIr.samples;
      // TODOG check if the names leak
    }
  }

  void resampleAndLoadIR(IRBundle &b) {
    mIRLoaded = false;
    clearLoadedIR();

    if (b.path.GetLength() != 0 && b.samples == nullptr) {
      // Load the wav file if the to be loaded IR is custom
      loadWave(b);
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
        for (int ch = 0; ch < mChannelCount; ch++) {
          mConvolvers[ch]->init(128, 1024 * 4, outBuffer, outSamples);
        }
      }
      else if (b.channelCount == mChannelCount) {
        mConvolvers[c]->init(128, 1024 * 4, outBuffer, outSamples);
      }
      delete[] outBuffer;
    }
    mCabShared.loadedIr = b;
    mIRLoaded = true;
  }

  static void loadWave(IRBundle &b) {
    drwav wav;
    if (!drwav_init_file(&wav, b.path.Get(), nullptr)) {
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
  }

  void serializeAdditional(nlohmann::json& serialized) override {
    serialized["irName"] = mCabShared.loadedIr.name.Get();
    bool custom = mCabShared.loadedIr.path.GetLength() != 0;
    serialized["customIR"] = custom;
    serialized["path"] = custom ? mCabShared.loadedIr.path.Get() : "";
  }

  void deserializeAdditional(nlohmann::json& serialized) override {
    try {
      IRBundle load;
      const string name = serialized.at("irName");
      load.name.Set(name.c_str());
      const bool customIR = serialized.at("customIR");
      if (customIR) {
        const string path = serialized.at("path");
        load.path.Set(path.c_str());
      }
      else {
        for (int i = 0; i < InternalIRsCount; i++) {
          // Go look for the right internal IR
          if (strncmp(load.name.Get(), InternalIRs[i].name.Get(), 30) == 0) {
            load = InternalIRs[i];
            break;
          }
        }
      }
      resampleAndLoadIR(load);
    }
    catch (...) {}
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
    resampleAndLoadIR(InternalIRs[0]);
  }

  void deleteBuffers() override {
    Node::deleteBuffers();
    clearLoadedIR();
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

  void ProcessBlock(const int nFrames) override {
    if (!inputsReady() || mIsProcessed || byPass()) { return; }
    if (!mIRLoaded) {
      outputSilence();
      return;
    }
    shared.parameters[1]->update();

    sample** buffer = shared.socketsIn[0]->mConnectedTo->mParentBuffer;

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
    SimpleCabNodeUi* ui = new SimpleCabNodeUi(&shared);
    ui->registerSharedData(&mCabShared);
    mUi = ui;
    pGrahics->AttachControl(mUi);
    mUi->setColor(IColor(255, 150, 100, 100));
    mUi->setUp();
    mUiReady = true;
    
  }
};
