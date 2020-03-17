#pragma once
#include "../../node/Node.h"
#include "../simple_cab/WrappedConvolver.h"

namespace guitard {
  /**
   * Fairly similar to SimpleCabNode
   * Has a more complex UI in CabLibPopUp.h which uses the soundwoofer API
   * Also does fading between 2 Convolvers to reduce popping sounds when flipping through IRs
   */
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

    soundwoofer::async::Callback mCallback;


    bool mIsBlending = false;

  public:
    soundwoofer::SWImpulseShared mLoadedIr = InternalIRs[0]; // So we got some kind of ir going
    CabLibNode(NodeList::NodeInfo* info) {
      mInfo = info;
      mStereo = 0;
      addByPassParam();
      addStereoParam();

      // This is the main function called when changing the IR
      mCallback = std::make_shared<soundwoofer::async::CallbackFunc>(
        [&](soundwoofer::Status s) {
        mConvolver2->loadIR(
          mLoadedIr->samples,
          mLoadedIr->length,
          mLoadedIr->channels
        );
        mBlendPos = 0;
        mIsBlending = true;
      }
      );
    }

    /**
     * Will be called from the UI to start loading an ir
     */
    void loadIr(soundwoofer::SWImpulseShared ir) {
      if (mLoadedIr->file != ir->file) {
        // don't load if the Ir is the same
        mLoadedIr = ir;
        soundwoofer::async::ir::load(ir, mCallback, mSampleRate);

        // (*mCallback)(soundwoofer::ir::load(ir, mSampleRate)); // non async version
      }
    }


    void serializeAdditional(nlohmann::json& serialized) override {
      serialized["path"] = mLoadedIr->file;
      serialized["id"] = mLoadedIr->id;
    }

    void deserializeAdditional(nlohmann::json& serialized) override {
      try {
        if (!serialized.contains("path")) {
          return;
        }
        mLoadedIr = std::make_shared<soundwoofer::SWImpulse>();
        mLoadedIr->file = serialized.at("path").get<std::string>();
        if (serialized.contains("id")) {
          mLoadedIr->id = serialized.at("id").get<std::string>();
        }
        soundwoofer::async::ir::loadUnknown(&mLoadedIr, mCallback, mSampleRate);
      }
      catch (...) {
        WDBGMSG("Failed to load Cab node data!\n");
      }
    }

    void createBuffers() override {
      Node::createBuffers();
      mConvolver = new WrappedConvolver(mMaxBlockSize);
      mConvolver2 = new WrappedConvolver(mMaxBlockSize);
      mBlendBuffer = new sample * [mChannelCount];
      for (int c = 0; c < mChannelCount; c++) {
        mBlendBuffer[c] = new sample[mMaxBlockSize];
      }
      soundwoofer::SWImpulseShared& ir = mLoadedIr;
      WDBGMSG("Load ir");
      soundwoofer::ir::load(ir, mSampleRate);
      WDBGMSG("Load done load");
      mConvolver->loadIR(ir->samples, ir->length, ir->channels);
      WDBGMSG("In convolver");
      //soundwoofer::async::loadIR(ir, [&, ir](soundwoofer::Status status) {
      //  if (status == soundwoofer::SUCCESS) {
      //    if (mConvolver == nullptr) { return; }
      //    mConvolver->resampleAndLoadIR(ir->samples, ir->length, ir->sampleRate, ir->channels);
      //  }
      //});
    }

    void deleteBuffers() override {
      Node::deleteBuffers();
      delete mConvolver;
      delete mConvolver2;
      mConvolver = nullptr;
      mConvolver2 = nullptr;
      if (mBlendBuffer != nullptr) {
        for (int c = 0; c < mChannelCount; c++) {
          delete[] mBlendBuffer[c];
        }
        delete[] mBlendBuffer;
      }
    }

    /**
     * Take care of samplerate and channel changes directly since both will need the convolver to be reconstructed
     * This prevents that from happening twice
     */
    void OnReset(const int pSampleRate, const int pChannels, const bool force = false) override {
      if (pSampleRate != mSampleRate || pChannels != mChannelCount || force) {
        deleteBuffers();
        mSampleRate = pSampleRate;
        mChannelCount = pChannels;
        mBlendStep = 1.0f / (pSampleRate * mTransitionTime);
        createBuffers();
      }
    }

    void ProcessBlock(const int nFrames) override {
      if (byPass()) { return; }
      if (mConvolver == nullptr) {
        outputSilence();
        return;
      }
      mParameters[1].update(); // this is the stereo param
      mConvolver->mStereo = mStereo > 0.5 ? true : false;

      if (mIsBlending) { // Means we'll need to take care of 2 convolvers
        mConvolver2->mStereo = mConvolver->mStereo; // Sync the stereo flag
        mConvolver->ProcessBlock(
          mSocketsIn[0].mBuffer, mSocketsOut[0].mBuffer, nFrames
        );
        mConvolver2->ProcessBlock(
          mSocketsIn[0].mBuffer, mBlendBuffer, nFrames
        );
        for (int i = 0; i < nFrames; i++) {
          if (mBlendPos < 1.0) {
            mBlendPos += mBlendStep;
          }
          else {
            mBlendPos = 1.0;
          }
          for (int c = 0; c < mChannelCount; c++) {
            mSocketsOut[0].mBuffer[c][i] = mSocketsOut[0].mBuffer[c][i] * (1 - mBlendPos) + mBlendBuffer[c][i] * mBlendPos;
          }
        }
        if (mBlendPos >= 1.0) { // Blend is over
          mIsBlending = false;
          std::swap(mConvolver, mConvolver2);
        }
      }
      else { // Normal processing
        mConvolver->ProcessBlock(
          mSocketsIn[0].mBuffer, mSocketsOut[0].mBuffer, nFrames
        );
      }
    }

    String getLicense() override {
      String l = "\nDefault IRs provided by Soundwoofer\n";
      l += "Public Domain\n\n";
      l += WrappedConvolver::getLicense();
      return l;
    }
  };

  GUITARD_REGISTER_NODE(
    CabLibNode, "Cabinet Library", "Cabinets", "Cabinet library for quick browsing through IRs"
  )
}

#ifndef GUITARD_HEADLESS
#include "../../ui/NodeUi.h"
#include "./CabLibPopUp.h"

namespace guitard {
  /**
   * Most of the logic is in the popup
   */
  class CabLibNodeUi : public NodeUi {
    CabLibPopUp* mPopUp = nullptr;
    IVButtonControl* mEditButton = nullptr;
  public:
    CabLibNodeUi(Node* node, MessageBus::Bus* bus) : NodeUi(node, bus) {
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
    }

    void openSettings() {
      mPopUp = new CabLibPopUp(dynamic_cast<CabLibNode*>(mNode));
      GetUI()->AttachControl(mPopUp);
    }

    void OnDetached() override {
      GetUI()->RemoveControl(mEditButton);
      GetUI()->RemoveControl(mPopUp);
      NodeUi::OnDetached();
    }
  };

  GUITARD_REGISTER_NODE_UI(CabLibNode, CabLibNodeUi)
}
#endif
