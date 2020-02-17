#pragma once

#include "../../node/Node.h"
#include "../../../thirdparty/soundwoofer/soundwoofer.h"
#ifndef GUITARD_HEADLESS
  #include "filebrowse.h"
#endif
#include "./InternalIRs.h"
#include "./WrappedConvolver.h"
#include "../../types/files.h"

// TODOG figure out this swell stuff
#ifdef FillRect
#undef FillRect
#endif
#ifdef DrawText
#undef DrawText
#endif

namespace guitard {
  /** Handy little struct to load irs and so on */
  struct CabNodeSharedData {
    std::function<void(soundwoofer::SWImpulseShared)> callback;
    soundwoofer::SWImpulseShared loadedIr;
    // bool embedIr = false;
  };

#ifndef GUITARD_HEADLESS

  class SimpleCabNodeUi : public NodeUi {
    IText mBlocksizeText;
    IVButtonControl* mBrowseButton = nullptr;
    CabNodeSharedData* mCabShared = nullptr;
    IPopupMenu mMenu{ "Choose IR", {"Clean", "Air", "From File"}, [&](IPopupMenu* pMenu) {
      IPopupMenu::Item* itemChosen = pMenu->GetChosenItem();
      if (itemChosen) {
        const String text = itemChosen->GetText();
        if (text == "From File") {
          this->openFileDialog();
        }
        else {
          for (int i = 0; i < InternalIRsCount; i++) {
            if (InternalIRs[i]->name == text) {
              mCabShared->callback(InternalIRs[i]);
              break;
            }
          }
        }
      }
    } };

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
      mElements.add(mBrowseButton);
      shared->graphics->AttachControl(mBrowseButton);
    }


    void openFileDialog() const {
      const HWND handle = reinterpret_cast<HWND>(shared->graphics->GetWindow());
      const String result = WDL_ChooseFileForOpen( // TODOG LEAK: the string might leak
        handle, "Open IR", nullptr, nullptr,
        "Wave Files\0*.wav;*.WAV\0", "*.wav",
        true, false
      );
      if (!result.empty()) {
        WDBGMSG(result.c_str());
        soundwoofer::SWImpulseShared load(new soundwoofer::SWImpulse());
        load->file = result;
        load->name = File::getFilePart(result);
        load->source = soundwoofer::USER_SRC_ABSOLUTE;
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
      soundwoofer::SWImpulseShared load(new soundwoofer::SWImpulse());
      load->file = str;
      load->name = File::getFilePart(load->file);
      load->source = soundwoofer::USER_SRC_ABSOLUTE;
      mCabShared->callback(load);
      if (soundwoofer::file::isWaveName(load->name)) {
        mCabShared->callback(load);
      }
    }

    void Draw(IGraphics& g) override {
      NodeUi::Draw(g);
      if (mCabShared != nullptr) {
        g.DrawText(mBlocksizeText, mCabShared->loadedIr->name.c_str(), mRECT.GetVShifted(20));
      }
    }

    void OnDetached() override {
      NodeUi::OnDetached();
      shared->graphics->RemoveControl(mBrowseButton);
    }

    void registerSharedData(CabNodeSharedData* data) {
      mCabShared = data;
    }
  };
#endif

  class SimpleCabNode final : public Node {
    WrappedConvolver* mConvolver = nullptr;

    CabNodeSharedData mCabShared = { [&](soundwoofer::SWImpulseShared ir) {
        // This will be called from the gui when the IR changes
        soundwoofer::ir::load(ir, mSampleRate);
        mConvolver->loadIR(ir->samples, ir->length, ir->channels);
        mCabShared.loadedIr = ir;
      },
      InternalIRs[0] // This is the default IR when the cab gets created
    };

  public:
    SimpleCabNode(NodeList::NodeInfo* info) {
      shared.info = info;
      mStereo = 0;
      addByPassParam();
      addStereoParam();
      shared.parameters[1].y = -30;

    }

    void serializeAdditional(nlohmann::json& serialized) override {
      serialized["irName"] = mCabShared.loadedIr->name;
      bool custom = mCabShared.loadedIr->source != soundwoofer::EMBEDDED_SRC;
      serialized["customIR"] = custom;
      serialized["path"] = mCabShared.loadedIr->file;
    }

    void deserializeAdditional(nlohmann::json& serialized) override {
      try {
        soundwoofer::SWImpulseShared load(new soundwoofer::SWImpulse());
        if (!serialized.contains("irName")) {
          return;
        }
        load->name = serialized.at("irName").get<std::string>();
        const bool customIR = serialized.at("customIR");
        load->file = serialized.at("path").get<std::string>();
        load->source = soundwoofer::USER_SRC_ABSOLUTE;
        if (!customIR) {
          for (int i = 0; i < InternalIRsCount; i++) {
            // Go look for the right internal IR
            if (InternalIRs[i]->name == load->name) {
              load = InternalIRs[i];
              break;
            }
          }
        }
        mCabShared.callback(load);
      }
      catch (...) {
        WDBGMSG("Failed to load Cab node data!\n");
      }
    }

    void createBuffers() override {
      Node::createBuffers();
      mConvolver = new WrappedConvolver(mSampleRate, shared.maxBlockSize);
      soundwoofer::SWImpulseShared ir = mCabShared.loadedIr;
      soundwoofer::ir::load(ir, mSampleRate);
      if (mConvolver != nullptr) { // Since the lambda could return at a point were samplerate has changed we check for null
        mConvolver->loadIR(ir->samples, ir->length, ir->channels);
      }
      //soundwoofer::async::loadIR(ir, [&, ir](soundwoofer::Status status) {
      //  if (status == soundwoofer::SUCCESS) {
      //    if (mConvolver != nullptr) { // Since the lambda could return at a point were samplerate has changed we check for null
      //      mConvolver->resampleAndLoadIR(ir->samples, ir->length, ir->sampleRate, ir->channels);
      //    }
      //  }
      //});
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
      if (pSampleRate != mSampleRate || pChannels != mChannelCount  || force) {
        deleteBuffers();
        mSampleRate = pSampleRate;
        mChannelCount = pChannels;
        createBuffers();
      }
    }

    void ProcessBlock(const int nFrames) override {
      if (!inputsReady() || mIsProcessed || byPass()) { return; }
      shared.parameters[1].update();
      if (mConvolver == nullptr) {
        outputSilence();
        return;
      }
      mConvolver->mStereo = mStereo > 0.5 ? true : false;
      mConvolver->ProcessBlock(
        shared.socketsIn[0]->mConnectedTo[0]->mParentBuffer, mBuffersOut[0], nFrames
      );
      mIsProcessed = true;
    }

#ifndef GUITARD_HEADLESS
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
#endif

    String getLicense() override {
      return WrappedConvolver::getLicense();
    }
  };
}