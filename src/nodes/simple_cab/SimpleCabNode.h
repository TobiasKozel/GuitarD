#pragma once



#include "thirdparty/fftconvolver/TwoStageFFTConvolver.h"
#include "resample.h"
#include "config.h"
#include "src/node/Node.h"
#include "clean.h"

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
    g.DrawBitmap(mBitmap, mRECT, 1, &mBlend);
    g.DrawText(mBlocksizeText, mInfo.c_str(), mRECT);
  }


};

class SimpleCabNode : public Node {
  fftconvolver::TwoStageFFTConvolver convolver;
  fftconvolver::TwoStageFFTConvolver convolver2;
  WDL_Resampler mResampler;
  WDL_RESAMPLE_TYPE* resampledIR;
  WDL_RESAMPLE_TYPE selectedIr;
  WDL_RESAMPLE_TYPE* conversionBufferIn;
  WDL_RESAMPLE_TYPE* conversionBufferOut;
public:
  SimpleCabNode(std::string pType) : Node() {
    type = pType;
    resampledIR = nullptr;
    conversionBufferIn = nullptr;
    conversionBufferOut = nullptr;
    selectedIr = 0;
  }

  // TODO FIX THIS MESS
  void setup(int p_samplerate = 48000, int p_maxBuffer = 512, int p_channles = 2, int p_inputs = 1, int p_outputs = 1) {
    Node::setup(p_samplerate, p_maxBuffer, 2, 1, 1);
#ifdef FLOATCONV
    conversionBufferIn = new WDL_RESAMPLE_TYPE[p_maxBuffer];
    conversionBufferOut = new WDL_RESAMPLE_TYPE[p_maxBuffer];
#endif
    //ParameterCoupling* p = new ParameterCoupling("IR", &selectedIr, 0.0, 0.0, 2.0, 1.0);
    //parameters.Add(p);
    mResampler.SetMode(true, 0, true);
    mResampler.SetFilterParms();
    mResampler.SetFeedMode(true);
    mResampler.SetRates(48000, p_samplerate);
    WDL_RESAMPLE_TYPE* test;
    int inSamples = mResampler.ResamplePrepare(cleanIRLength, 1, &test);
    for (int i = 0; i < cleanIRLength; i++) {
      test[i] = cleanIR[i] * (48000 / p_samplerate) * 0.1;
    }
    resampledIR = new WDL_RESAMPLE_TYPE[cleanIRLength * ((48000 / p_samplerate))];
    int outSamples = mResampler.ResampleOut(resampledIR, inSamples, cleanIRLength, 1);
    convolver.init(128, 1024 * 4, resampledIR, outSamples);
    convolver2.init(128, 1024 * 4, resampledIR, outSamples);
    mStereo = 0;
    addByPassParam();
    addStereoParam();
  }

  ~SimpleCabNode() {
    delete resampledIR;
#ifdef FLOATCONV
    delete conversionBufferIn;
    delete conversionBufferOut;
#endif
  }

  void ProcessBlock(int nFrames) {
    if (!inputsReady() || isProcessed || byPass()) { return; }
    parameters.Get(1)->update();
    //int prev = (int)*(parameters[0]->value);
    //parameters[0]->update();
    //int cur = (int)*(parameters[0]->value);
    //if (prev != cur) {
    //  if (cur == 0) {
    //    convolver.init(64, cleanIR, 3000);
    //  }
    //  if (cur == 1) {
    //    convolver.init(64, stackIR, 4500);
    //  }
    //  if (cur == 2) {
    //    convolver.init(64, driveIR, 4200);
    //  }
    //}

    sample** buffer = inSockets.Get(0)->connectedTo->parentBuffer;

#ifdef FLOATCONV
    for (int i = 0; i < nFrames; i++) {
      conversionBufferIn[i] = buffer[0][i];
    }
    convolver.process(conversionBufferIn, conversionBufferOut, nFrames);
    for (int i = 0; i < nFrames; i++) {
      outputs[0][0][i] = conversionBufferOut[i];
    }
    if (mStereo) {
      for (int i = 0; i < nFrames; i++) {
        conversionBufferIn[i] = buffer[1][i];
      }
      convolver2.process(conversionBufferIn, conversionBufferOut, nFrames);
      for (int i = 0; i < nFrames; i++) {
        outputs[0][1][i] = conversionBufferOut[i];
      }
    }
    else {
      for (int i = 0; i < nFrames; i++) {
        outputs[0][1][i] = outputs[0][0][i];
      }
    }
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
    isProcessed = true;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    mUi = new SimpleCabNodeUi(NodeUiParam{
      pGrahics,
      PNGGENERICBG_FN,
      &X, &Y,
      &parameters,
      &inSockets,
      &outSockets,
      this
    });
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    uiReady = true;
  }
};
