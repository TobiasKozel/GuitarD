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
    std::function<void(SoundWoofer::SWImpulseShared)> callback;
    SoundWoofer::SWImpulseShared loadedIr;
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
        const std::string text = itemChosen->GetText();
        if (text == "From File") {
          this->openFileDialog();
        }
        else {
          for (int i = 0; i < InternalIRsCount; i++) {
            if (InternalIRs[i]->name == text) {
              mCabShared->callback(InternalIRs[i]);
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
      const std::string result = WDL_ChooseFileForOpen( // TODOG LEAK: the string might leak
        handle, "Open IR", nullptr, nullptr,
        "Wave Files\0*.wav;*.WAV\0", "*.wav",
        true, false
      );
      if (result.empty()) {
        WDBGMSG(result.c_str());
        SoundWoofer::SWImpulseShared load(new SoundWoofer::SWImpulse());
        load->file = result;
        load->name = File::getFilePart(result);
        load->source = SoundWoofer::USER_SRC_ABSOLUTE;
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
      SoundWoofer::SWImpulseShared load(new SoundWoofer::SWImpulse());
      load->file = str;
      load->name = File::getFilePart(load->file);
      load->source = SoundWoofer::USER_SRC_ABSOLUTE;
      mCabShared->callback(load);
      if (SoundWoofer::isWaveName(load->name)) {
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

    CabNodeSharedData mCabShared = {
      // This will be called from the gui when the IR changes
      [&](SoundWoofer::SWImpulseShared ir) { 
      SoundWoofer::instance().loadIR(ir, [&](SoundWoofer::Status status) {
        if (status == SoundWoofer::SUCCESS) {
          mCabShared.loadedIr = ir;
          mConvolver->resampleAndLoadIR(ir->samples, ir->length, ir->sampleRate, ir->channels);
        }
      }); },
      InternalIRs[0] // This is the default IR when the cab gets created
    };

  public:
    SimpleCabNode(const std::string pType) {
      shared.type = pType;
      mStereo = 0;
      addByPassParam();
      addStereoParam();
      shared.parameters[1].y = -30;

    }

    void serializeAdditional(nlohmann::json& serialized) override {
      //serialized["irName"] = mCabShared.loadedIr.name.get();
      //bool custom = mCabShared.loadedIr.path.getLength() != 0;
      //serialized["customIR"] = custom;
      //serialized["path"] = custom ? mCabShared.loadedIr.path.get() : "";
    }

    void deserializeAdditional(nlohmann::json& serialized) override {
      try {
        //IRBundle load;
        //if (!serialized.contains("irName")) {
        //  return;
        //}
        //const std::string name = serialized.at("irName");
        //load.name.set(name.c_str());
        //const bool customIR = serialized.at("customIR");
        //if (customIR) {
        //  const std::string path = serialized.at("path");
        //  load.path.set(path.c_str());
        //}
        //else {
        //  for (int i = 0; i < InternalIRsCount; i++) {
        //    // Go look for the right internal IR
        //    if (strncmp(load.name.get(), InternalIRs[i].name.get(), 30) == 0) {
        //      load = InternalIRs[i];
        //      break;
        //    }
        //  }
        //}
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
      mConvolver = nullptr;
    }

    void OnSamplerateChanged(const int pSampleRate) override {
      deleteBuffers();
      mSampleRate = pSampleRate;
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

    std::string getLicense() override {
      return WrappedConvolver::getLicense();
    }
  };
}