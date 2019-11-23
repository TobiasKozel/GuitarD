#pragma once
#include "thirdparty/fftconvolver/TwoStageFFTConvolver.h"
#include "resample.h"
#include "src/node/Node.h"
#include "clean.h"
#include "thirdparty/threadpool.h"

// #define useThreadPool
// #define useOpenMP

#ifdef useOpenMP
#include <omp.h>
#endif

class FileBrowser : public IDirBrowseControlBase
{
private:
  WDL_String mLabel;
  IBitmap mBitmap;
public:
  FileBrowser(const IRECT& bounds)
    : IDirBrowseControlBase(bounds, ".wav")
  {
    WDL_String path;
    //    DesktopPath(path);
    path.Set(__FILE__);
    path.remove_filepart();
#ifdef OS_WIN
    path.Append("\\resources\\img\\");
#else
    path.Append("/resources/img/");
#endif
    AddPath(path.Get(), "");

    mLabel.Set("Click here to browse IR files...");
  }

  void Draw(IGraphics& g) override
  {
    g.FillRect(COLOR_TRANSLUCENT, mRECT);

  }

  void OnMouseDown(float x, float y, const IMouseMod& mod) override
  {
    SetUpMenu();

    GetUI()->CreatePopupMenu(*this, mMainMenu, x, y);
  }

  void OnPopupMenuSelection(IPopupMenu* pSelectedMenu, int valIdx) override
  {
    if (pSelectedMenu)
    {
      IPopupMenu::Item* pItem = pSelectedMenu->GetChosenItem();
      WDL_String* pStr = mFiles.Get(pItem->GetTag());
      mLabel.Set(pStr);
      mBitmap = GetUI()->LoadBitmap(pStr->Get());
      SetTooltip(pStr->Get());
      SetDirty(false);
    }
  }
};

class SimpleCabNodeUi : public NodeUi {
  iplug::igraphics::IText mBlocksizeText;
  string mInfo;
public:
  SimpleCabNodeUi(NodeShared* param) : NodeUi(param) {
    mInfo = "None";
    mBlocksizeText = DEBUG_FONT;
  }

  void OnDrop(const char* str) override {
    mInfo = "Path: " + string(str);
    mDirty = true;
  }

  void Draw(IGraphics& g) override {
    NodeUi::Draw(g);
    g.DrawText(mBlocksizeText, mInfo.c_str(), mRECT);
  }


};

class SimpleCabNode final : public Node {
  fftconvolver::TwoStageFFTConvolver* mConvolvers[8];
  WDL_Resampler mResampler;
  WDL_RESAMPLE_TYPE* mResampledIR;
  WDL_RESAMPLE_TYPE** mConversionBufferIn;
  WDL_RESAMPLE_TYPE** mConversionBufferOut;

#ifdef useThreadPool
  ctpl::thread_pool tPool;
#endif
  
public:
  SimpleCabNode(std::string pType) : Node() {
    mType = pType;
    mResampledIR = nullptr;
    mConversionBufferIn = nullptr;
    mConversionBufferOut = nullptr;
  }

  // TODOG FIX THIS MESS
  void setup(MessageBus::Bus* pBus, int pSamplerate = 48000, int pMaxBuffer = MAX_BUFFER, int pChannles = 2, int pInputs = 1, int pOutputs = 1) override {
    Node::setup(pBus, pSamplerate, pMaxBuffer, 2, 1, 1);

    mStereo = 0;
    addByPassParam();
    addStereoParam();
#ifdef useThreadPool
    tPool.resize(2);
#endif
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
    mResampler.SetMode(true, 0, true);
    mResampler.SetFilterParms();
    mResampler.SetFeedMode(true);
    mResampler.SetRates(48000, mSampleRate);
    WDL_RESAMPLE_TYPE* test;
    const int inSamples = mResampler.ResamplePrepare(cleanIRLength, 1, &test);
    for (int i = 0; i < cleanIRLength; i++) {
      test[i] = cleanIR[i] * (48000.f / static_cast<float>(mSampleRate) ) * 0.2;
    }
    mResampledIR = new WDL_RESAMPLE_TYPE[
      static_cast<size_t>(ceil(cleanIRLength * ((mSampleRate / 48000.f))))
    ];
    const int outSamples = mResampler.ResampleOut(mResampledIR, inSamples, cleanIRLength, 1);
    for (int c = 0; c < mChannelCount; c++) {
      mConvolvers[c]->init(128, 1024 * 4, mResampledIR, outSamples);
    }
  }

  void deleteBuffers() override {
    Node::deleteBuffers();
    delete mResampledIR;
    mResampledIR = nullptr;
    for (int c = 0; c < mChannelCount; c++) {
      delete mConvolvers[c];
    }
#ifdef FLOATCONV
    for (int c = 0; c < mChannelCount; c++) {
      delete mConversionBufferIn[c];
      delete mConversionBufferOut[c];
    }
    delete mConversionBufferIn;
    delete mConversionBufferOut;
    mConversionBufferIn = nullptr;
    mConversionBufferOut = nullptr;
#endif
  }

  void ProcessBlock(int nFrames) {
    if (!inputsReady() || mIsProcessed || byPass()) { return; }
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
    Node::setupUi(pGrahics);
    mUi->setColor(IColor(255, 150, 100, 100));
  }
};
