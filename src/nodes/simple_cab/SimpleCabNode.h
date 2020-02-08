#pragma once

#include "src/node/Node.h"
#include "filebrowse.h"
#include "InternalIRs.h"
#include "WrappedConvolver.h"

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
    std::function<void(IRBundle)> callback;
    IRBundle loadedIr = InternalIRs[0];
    bool embedIr = false;
  };

#ifndef GUITARD_HEADLESS

  class SimpleCabNodeUi : public NodeUi {
    IText mBlocksizeText;
    IVButtonControl* mBrowseButton = nullptr;
    CabNodeSharedData* mCabShared = nullptr;
    IPopupMenu mMenu{ "Choose IR", {"Clean", "Air", "From File"}, [&](IPopupMenu* pMenu) {
      IPopupMenu::Item* itemChosen = pMenu->GetChosenItem();
      if (itemChosen) {
        const char* text = itemChosen->GetText();
        if (strncmp(text, "From File", 10) == 0) {
          this->openFileDialog();
        }
        else {
          for (int i = 0; i < InternalIRsCount; i++) {
            if (strncmp(InternalIRs[i].name.get(), text, 30) == 0) {
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
      char* result = WDL_ChooseFileForOpen(
        handle, "Open IR", nullptr, nullptr,
        //"Wave Files\0*.wav;*.WAV\0AIFF Files\0*.aiff;*.AIFF\0", "*.wav",
        "Wave Files\0*.wav;*.WAV\0", "*.wav",
        true, false
      );
      if (result != nullptr) {
        WDBGMSG(result);
        IRBundle load;
        load.path.set(result);
        load.name.set(load.path.getFilePart());
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
      load.path.set(str);
      load.name.set(load.path.getFilePart());
      if (strncmp(load.name.getExt(), ".wav", 4) == 0) {
        mCabShared->callback(load);
      }
    }

    void Draw(IGraphics& g) override {
      NodeUi::Draw(g);
      if (mCabShared != nullptr) {
        g.DrawText(mBlocksizeText, mCabShared->loadedIr.name.get(), mRECT.GetVShifted(20));
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

    CabNodeSharedData mCabShared = { [&](IRBundle ir) {
      this->mCabShared.loadedIr = ir;
      this->mConvolver->resampleAndLoadIR(&ir);
    }, InternalIRs[0], false };

  public:
    SimpleCabNode(const std::string pType) {
      shared.type = pType;
      mStereo = 0;
      addByPassParam();
      addStereoParam();
      shared.parameters[1].y = -30;

    }

    void serializeAdditional(nlohmann::json& serialized) override {
      serialized["irName"] = mCabShared.loadedIr.name.get();
      bool custom = mCabShared.loadedIr.path.getLength() != 0;
      serialized["customIR"] = custom;
      serialized["path"] = custom ? mCabShared.loadedIr.path.get() : "";
    }

    void deserializeAdditional(nlohmann::json& serialized) override {
      try {
        IRBundle load;
        if (!serialized.contains("irName")) {
          return;
        }
        const std::string name = serialized.at("irName");
        load.name.set(name.c_str());
        const bool customIR = serialized.at("customIR");
        if (customIR) {
          const std::string path = serialized.at("path");
          load.path.set(path.c_str());
        }
        else {
          for (int i = 0; i < InternalIRsCount; i++) {
            // Go look for the right internal IR
            if (strncmp(load.name.get(), InternalIRs[i].name.get(), 30) == 0) {
              load = InternalIRs[i];
              break;
            }
          }
        }
        mCabShared.loadedIr = load;
        mConvolver->resampleAndLoadIR(&load);
      }
      catch (...) {
        WDBGMSG("Failed to load Cab node data!\n");
      }
    }

    void createBuffers() override {
      Node::createBuffers();
      mConvolver = new WrappedConvolver(mSampleRate, shared.maxBlockSize);
      mConvolver->resampleAndLoadIR(&mCabShared.loadedIr);
    }

    void deleteBuffers() override {
      mConvolver->unloadWave(&mCabShared.loadedIr);
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