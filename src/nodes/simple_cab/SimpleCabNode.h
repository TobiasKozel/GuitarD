#pragma once
#include "thirdparty/fftconvolver/TwoStageFFTConvolver.h"
#include "resample.h"
#include "config.h"
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
  SimpleCabNodeUi(NodeUiParam param) : NodeUi(param) {
    mInfo = "None";
    mBlocksizeText = DEBUGFONT;
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

class SimpleCabNode : public Node {
  fftconvolver::TwoStageFFTConvolver* convolvers[8];
  WDL_Resampler mResampler;
  WDL_RESAMPLE_TYPE* resampledIR;
  WDL_RESAMPLE_TYPE selectedIr;
  WDL_RESAMPLE_TYPE** conversionBufferIn;
  WDL_RESAMPLE_TYPE** conversionBufferOut;

#ifdef useThreadPool
  ctpl::thread_pool tPool;
#endif
  
public:
  SimpleCabNode(std::string pType) : Node() {
    mType = pType;
    resampledIR = nullptr;
    conversionBufferIn = nullptr;
    conversionBufferOut = nullptr;
    selectedIr = 1;
  }

  // TODOG FIX THIS MESS
  void setup(MessageBus::Bus* pBus, int p_samplerate = 48000, int p_maxBuffer = MAX_BUFFER, int p_channles = 2, int p_inputs = 1, int p_outputs = 1) {
    Node::setup(pBus, p_samplerate, p_maxBuffer, 2, 1, 1);

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
      convolvers[c] = new fftconvolver::TwoStageFFTConvolver();
    }
#ifdef FLOATCONV
    conversionBufferIn = new WDL_RESAMPLE_TYPE * [mChannelCount];
    conversionBufferOut = new WDL_RESAMPLE_TYPE * [mChannelCount];
    for (int c = 0; c < mChannelCount; c++) {
      conversionBufferIn[c] = new WDL_RESAMPLE_TYPE[mMaxBuffer];
      conversionBufferOut[c] = new WDL_RESAMPLE_TYPE[mMaxBuffer];
    }
#endif
    mResampler.SetMode(true, 0, true);
    mResampler.SetFilterParms();
    mResampler.SetFeedMode(true);
    mResampler.SetRates(48000, mSampleRate);
    WDL_RESAMPLE_TYPE* test;
    int inSamples = mResampler.ResamplePrepare(cleanIRLength, 1, &test);
    for (int i = 0; i < cleanIRLength; i++) {
      test[i] = cleanIR[i] * (48000.f / (float) mSampleRate ) * 0.2;
    }
    resampledIR = new WDL_RESAMPLE_TYPE[ceil(cleanIRLength * ((mSampleRate / 48000.f)))];
    int outSamples = mResampler.ResampleOut(resampledIR, inSamples, cleanIRLength, 1);
    for (int c = 0; c < mChannelCount; c++) {
      convolvers[c]->init(128, 1024 * 4, resampledIR, outSamples);
    }
  }

  void deleteBuffers() override {
    Node::deleteBuffers();
    delete resampledIR;
    resampledIR = nullptr;
    for (int c = 0; c < mChannelCount; c++) {
      delete convolvers[c];
    }
#ifdef FLOATCONV
    for (int c = 0; c < mChannelCount; c++) {
      delete conversionBufferIn[c];
      delete conversionBufferOut[c];
    }
    delete conversionBufferIn;
    delete conversionBufferOut;
    conversionBufferIn = nullptr;
    conversionBufferOut = nullptr;
#endif
  }

  void ProcessBlock(int nFrames) {
    if (!inputsReady() || mIsProcessed || byPass()) { return; }
    mParameters.Get(1)->update();

    sample** buffer = mSocketsIn.Get(0)->connectedTo->parentBuffer;

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
      conversionBufferIn[0][i] = buffer[0][i];
    }
    convolvers[0]->process(conversionBufferIn[0], conversionBufferOut[0], nFrames);
    for (int i = 0; i < nFrames; i++) {
      mBuffersOut[0][0][i] = conversionBufferOut[0][i];
    }

    if (!mStereo) {
      for (int i = 0; i < nFrames; i++) {
        mBuffersOut[0][1][i] = mBuffersOut[0][0][i];
      }
    }
    else {
      for (int i = 0; i < nFrames; i++) {
        conversionBufferIn[1][i] = buffer[1][i];
      }
      convolvers[1]->process(conversionBufferIn[1], conversionBufferOut[1], nFrames);
      for (int i = 0; i < nFrames; i++) {
        mBuffersOut[0][1][i] = conversionBufferOut[1][i];
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
    mUi = new SimpleCabNodeUi(NodeUiParam{
      mBus, pGrahics,
      300, 300,
      &mX, &mY, &mParameters, &mSocketsIn, &mSocketsOut, this
    });
    mUi->setColor(IColor(255, 150, 100, 100));
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    mUiReady = true;
  }
};
