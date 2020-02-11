#pragma once
#include "../../node/Node.h"

#include "./CabLibPopUp.h"

namespace guitard {
#ifndef GUITARD_HEADLESS
  class CabLibNodeUi : public NodeUi {
    // ScrollViewControl* test = nullptr;
    CabLibNodeSharedData* mCabShared = nullptr;
    CabLibPopUp* mPopUp = nullptr;
    IVButtonControl* mEditButton = nullptr;
  public:
    CabLibNodeUi(NodeShared* param) : NodeUi(param) {
    }

    void setUpControls() override {
      NodeUi::setUpControls();
      mEditButton = new IVButtonControl({ mTargetRECT.L + 50, mTargetRECT.T + 130, mTargetRECT.R - 50, mTargetRECT.B - 20 },
        [&](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        this->openSettings();
      }, "Browse", DEFAULT_STYLE, true, false
      );
      mElements.add(mEditButton);
      GetUI()->AttachControl(mEditButton);
      //test = new ScrollViewControl(mTargetRECT.GetPadded(-20));
      //test->setDoDragScroll(false);
      //mElements.Add(test);
      //IVButtonControl* editButton = new IVButtonControl(IRECT(0, 0, 100, 30), [&](IControl* pCaller) {
      //  SplashClickActionFunc(pCaller);
      //  this->openSettings();
      //}, "EDIT", DEFAULT_STYLE, true, false);
      //test->appendChild(editButton);
      //PlaceHolder* t = new PlaceHolder(IRECT(0, 0, 100, 20), "Element 1");
      //test->appendChild(t);
      //t = new PlaceHolder(IRECT(0, 0, 130, 60), "Element 2");
      //test->appendChild(t);
      //t = new PlaceHolder(IRECT(0, 0, 130, 40), "Element 3");
      //test->appendChild(t);
      //IVKnobControl* c = new IVKnobControl(
      //  IRECT(0, 0, 120, 120), 3
      //);
      //test->appendChild(c);
      //t = new PlaceHolder(IRECT(0, 0, 80, 80), "Element 4");
      //test->appendChild(t);
      //t = new PlaceHolder(IRECT(0, 0, 400, 20), "Element 5");
      //test->appendChild(t);
      //shared->graphics->AttachControl(test);
    }

    void openSettings() {
      mPopUp = new CabLibPopUp(mCabShared);
      GetUI()->AttachControl(mPopUp);
    }

    void OnDetached() override {
      GetUI()->RemoveControl(mEditButton);
      GetUI()->RemoveControl(mPopUp);
      NodeUi::OnDetached();
    }

    void registerSharedData(CabLibNodeSharedData* data) {
      mCabShared = data;
    }
  };
#endif

  class CabLibNode final : public Node {
    /** Time in seconds to use for blending between convolvers */
    const sample mTransitionTime = 0.1;
    sample mBlendStep = 0;
    sample mBlendPos = 0;
    sample** mBlendBuffer = nullptr;
    /** Primary convolver */
    WrappedConvolver* mConvolver = nullptr;
    /** Secondary convolver only used for blending */
    WrappedConvolver* mConvolver2 = nullptr;

    bool mIsBlending = false;

    CabLibNodeSharedData mCabShared = { [&](SoundWoofer::SWImpulseShared ir) {
      if (mCabShared.loadedIr->file == ir->file) {
        // don't load if the Ir is the same;
        SoundWoofer::instance().loadIR(ir, [&](SoundWoofer::Status status) {
          if (status == SoundWoofer::SUCCESS) {
            mCabShared.loadedIr = ir;
            mConvolver->resampleAndLoadIR(ir->samples, ir->length, ir->sampleRate, ir->channels);
            mBlendPos = 0;
            mIsBlending = true;
          }
        });
      }
      },std::make_shared<SoundWoofer::SWImpulse>()
    };
  public:
    CabLibNode(const std::string pType) {
      shared.type = pType;
      mStereo = 0;
      addByPassParam();
      addStereoParam();
      shared.parameters[1].y = -30;
    }


    void serializeAdditional(nlohmann::json& serialized) override {
      // serialized["path"] = mCabShared.loadedIr.path.get();
    }
    void deserializeAdditional(nlohmann::json& serialized) override {
      try {
        //IRBundle load;
        //if (!serialized.contains("path")) {
        //  return;
        //}
        //const std::string path = serialized.at("path");
        //load.path.set(path.c_str());
        //load.name.set(load.path.getFilePart());
        //mCabShared.loadedIr = load;
        //mConvolver->resampleAndLoadIR(&load);
      }
      catch (...) {
        WDBGMSG("Failed to load Cab node data!\n");
      }
    }

    void createBuffers() override {
      Node::createBuffers();
      mConvolver = new WrappedConvolver(mSampleRate, shared.maxBlockSize);
      mConvolver2 = new WrappedConvolver(mSampleRate, shared.maxBlockSize);
      mBlendBuffer = new sample * [mChannelCount];
      for (int c = 0; c < mChannelCount; c++) {
        mBlendBuffer[c] = new sample[shared.maxBlockSize];
      }
      SoundWoofer::SWImpulseShared& ir = mCabShared.loadedIr;
      SoundWoofer::instance().loadIR(ir, [&](SoundWoofer::Status status) {
        if (status == SoundWoofer::SUCCESS) {
          mConvolver->resampleAndLoadIR(ir->samples, ir->length, ir->sampleRate, ir->channels);
        }
      });
    }

    void deleteBuffers() override {
      Node::deleteBuffers();
      delete mConvolver;
      delete mConvolver2;
      mConvolver = nullptr;
      mConvolver2 = nullptr;
      for (int c = 0; c < mChannelCount; c++) {
        delete[] mBlendBuffer[c];
      }
      delete[] mBlendBuffer;
    }

    void OnSamplerateChanged(const int pSampleRate) override {
      deleteBuffers();
      mSampleRate = pSampleRate;
      mBlendStep = 1.0f / (pSampleRate * mTransitionTime);
      createBuffers();
    }

    void ProcessBlock(const int nFrames) override {
      if (!inputsReady() || mIsProcessed || byPass()) { return; }
      shared.parameters[1].update();
      if (mConvolver == nullptr) {
        outputSilence();
        return;
      }
      mConvolver->mStereo = mStereo > 0.5 ? true : false;

      if (mIsBlending) {
        mConvolver2->mStereo = mConvolver->mStereo;
        mConvolver->ProcessBlock(
          shared.socketsIn[0]->mConnectedTo[0]->mParentBuffer, mBuffersOut[0], nFrames
        );
        mConvolver2->ProcessBlock(
          shared.socketsIn[0]->mConnectedTo[0]->mParentBuffer, mBlendBuffer, nFrames
        );
        for (int i = 0; i < nFrames; i++) {
          if (mBlendPos < 1.0) {
            mBlendPos += mBlendStep;
          }
          else {
            mBlendPos = 1.0;
          }
          for (int c = 0; c < mChannelCount; c++) {
            mBuffersOut[0][c][i] = mBuffersOut[0][c][i] * (1 - mBlendPos) + mBlendBuffer[c][i] * mBlendPos;
          }
        }
        if (mBlendPos >= 1.0) {
          mIsBlending = false;
          WrappedConvolver* swap = mConvolver;
          mConvolver = mConvolver2;
          mConvolver2 = swap;
        }
      }
      else {
        mConvolver->ProcessBlock(
          shared.socketsIn[0]->mConnectedTo[0]->mParentBuffer, mBuffersOut[0], nFrames
        );
      }
      mIsProcessed = true;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(IGraphics* pGraphics) override {
      shared.graphics = pGraphics;
      CabLibNodeUi* ui = new CabLibNodeUi(&shared);
      ui->registerSharedData(&mCabShared);
      mUi = ui;
      pGraphics->AttachControl(mUi);
      mUi->setColor(IColor(255, 150, 100, 100));
      mUi->setUp();
      mUiReady = true;
    }
#endif

    std::string getLicense() override {
      std::string l = "\nDefault IRs provided by Soundwoofer\n";
      l += "Public Domain\n\n";
      l += WrappedConvolver::getLicense();
      return l;
    }
  };
}