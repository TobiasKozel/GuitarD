#pragma once
#include "../../main/Node.h"
#include "../../types/GConvolver.h"


#include "../../../thirdparty/SpeexResampler.hpp"

#include "../../../../resources/hardcoded_irs.hpp"
namespace guitard {
  /**
   * Fairly similar to SimpleCabNode
   * Has a more complex UI in CabLibPopUp.h which uses the soundwoofer API
   * Also does fading between 2 Convolvers to reduce popping sounds when flipping through IRs
   */
  class CabLibNode final : public Node {
    WrappedConvolver* mConvolver = nullptr;
    enum IrType {
      TypeNone = 0,
      TypeA,
      TypeB
    } mIrType = TypeNone;
  public:
    CabLibNode() {
      mStereo = 0;
      addByPassParam();
      addStereoParam();
    }



    void serializeAdditional(nlohmann::json& serialized) override {
    }

    void deserializeAdditional(nlohmann::json& serialized) override {
      try {
        if (!serialized.contains("path")) {
          return;
        }
        if (serialized["path"] == "5d6ed046-903f-4a76-904d-68fa25f91737") {
          mIrType = TypeA;
        } else {
          mIrType = TypeB;
        }
        deleteBuffers();
        createBuffers();
      }
      catch (...) {
        WDBGMSG("Failed to load Cab node data!\n");
      }
    }

    void createBuffers() override {
      if (mIrType == TypeNone) {
        return;
      }

      Node::createBuffers();
      mConvolver = new WrappedConvolver(mMaxBlockSize);
      unsigned int srcLength = mIrType == TypeA ? IR_TYPE_A_LEN : IR_TYPE_B_LEN;
      const float* srcBuf = mIrType == TypeA ? IR_TYPE_A : IR_TYPE_B;
      
      mConvolver->mStereo = true;

      if (IR_RATE == mSampleRate) {
        const float* srcMultiBuf[] = { srcBuf };
        mConvolver->loadIR(srcMultiBuf, srcLength, 1);
      } else {

        const double dstRate = mSampleRate;
        const double srcRate = IR_RATE;

        speexport::SpeexResampler resampler;
        int res;
        resampler.init(1, srcRate, dstRate, 6, &res);
        int latency = resampler.get_input_latency() + resampler.get_output_latency();
        const unsigned int dstLengthOrg = int(dstRate / srcRate * (double) srcLength) + 50; // add some leeway
        unsigned int dstLength = dstLengthOrg;
        float* dstBuf = new float[dstLengthOrg];
        resampler.skip_zeros();
        resampler.process(0, srcBuf, &srcLength, dstBuf, &dstLength);

        const float scale = srcRate / dstRate;
        for (int i = 0; i < dstLength; i++) {
          dstBuf[i] *= scale;
        }

        const float* srcMultiBuf[] = { dstBuf };
        mConvolver->loadIR(srcMultiBuf, dstLength, 1);
        delete[] dstBuf;
      }
    }

    void deleteBuffers() override {
      Node::deleteBuffers();
      delete mConvolver;
      mConvolver = nullptr;
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
        createBuffers();
      }
    }

    void ProcessBlock(const int nFrames) override {
      if (byPass()) { return; }
      if (mConvolver == nullptr) {
        outputSilence();
        return;
      }
      //mParameters[1].update(); // this is the stereo param
      //mConvolver->mStereo = mStereo > 0.5 ? true : false;

      mConvolver->ProcessBlock(
        mSocketsIn[0].mBuffer, mSocketsOut[0].mBuffer, nFrames
      );
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
#include "../../ui/elements/NodeUi.h"
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
