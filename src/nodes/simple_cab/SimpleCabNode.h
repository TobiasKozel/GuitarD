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
  class SimpleCabNode final : public Node {
    WrappedConvolver* mConvolver = nullptr;

  public:
    soundwoofer::SWImpulseShared mLoadedIr = InternalIRs[0];
    SimpleCabNode(NodeList::NodeInfo* info) {
      mInfo = info;
      mStereo = 0;
      addByPassParam();
      addStereoParam();
    }

    void loadIr(soundwoofer::SWImpulseShared ir) {
      // This will be called from the gui when the IR changes
      soundwoofer::ir::load(ir, mSampleRate);
      mConvolver->loadIR(ir->samples, ir->length, ir->channels);
      mLoadedIr = ir;
    }

    void serializeAdditional(nlohmann::json& serialized) override {
      serialized["irName"] = mLoadedIr->name;
      bool custom = mLoadedIr->source != soundwoofer::EMBEDDED_SRC;
      serialized["customIR"] = custom;
      serialized["path"] = mLoadedIr->file;
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
        loadIr(load);
      }
      catch (...) {
        WDBGMSG("Failed to load Cab node data!\n");
      }
    }

    void createBuffers() override {
      Node::createBuffers();
      mConvolver = new WrappedConvolver(mMaxBlockSize);
      soundwoofer::SWImpulseShared ir = mLoadedIr;
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
      if (pSampleRate != mSampleRate || pChannels != mChannelCount || force) {
        deleteBuffers();
        mSampleRate = pSampleRate;
        mChannelCount = pChannels;
        createBuffers();
      }
    }

    void ProcessBlock(const int nFrames) override {
      if (byPass()) { return; }
      mParameters[1].update();
      if (mConvolver == nullptr) {
        outputSilence();
        return;
      }
      mConvolver->mStereo = mStereo > 0.5 ? true : false;
      mConvolver->ProcessBlock(
        mSocketsIn[0].mBuffer, mSocketsOut[0].mBuffer, nFrames
      );
    }

    String getLicense() override {
      return WrappedConvolver::getLicense();
    }
  };

  GUITARD_REGISTER_NODE(
    SimpleCabNode, "Simple Cabinet", "Cabinets", "Can load in IRs up to 10 seconds"
  )
}

#ifndef GUITARD_HEADLESS
#include "../../ui/NodeUi.h"
namespace guitard {
  class SimpleCabNodeUi : public NodeUi {
    SimpleCabNode* mCab = nullptr; // This is just a cast of the node itself
    IText mBlocksizeText;
    IVButtonControl* mBrowseButton = nullptr;
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
              mCab->loadIr(InternalIRs[i]);
              break;
            }
          }
        }
      }
    } };

  public:
    SimpleCabNodeUi(Node* node, MessageBus::Bus* bus) : NodeUi(node, bus) {
      mCab = dynamic_cast<SimpleCabNode*>(node);
      if (mCab == nullptr) {
        assert(false); // Shouldn't really happen
      }
      mBlocksizeText = DEBUG_FONT;
    }

    void setUpControls() override {
      NodeUi::setUpControls();
      const IRECT button{ mTargetRECT.L + 50, mTargetRECT.T + 130, mTargetRECT.R - 50, mTargetRECT.B - 20 };
      mBrowseButton = new IVButtonControl(button, [&](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        float x, y;
        GetUI()->GetMouseDownPoint(x, y);
        GetUI()->CreatePopupMenu(*pCaller, mMenu, x, y);

      }, "Select IR", DEFAULT_STYLE, true, false);
      mElements.add(mBrowseButton);
      GetUI()->AttachControl(mBrowseButton);
    }


    void openFileDialog() {
      const HWND handle = reinterpret_cast<HWND>(GetUI()->GetWindow());
      char* path = WDL_ChooseFileForOpen( // TODOG LEAK: the string might leak
        handle, "Open IR", nullptr, nullptr,
        "Wave Files\0*.wav;*.WAV\0", "*.wav",
        true, false
      );
      if (path != nullptr) {
        const String result = path;
        free(path);
        WDBGMSG(result.c_str());
        soundwoofer::SWImpulseShared load(new soundwoofer::SWImpulse());
        load->file = result;
        load->name = File::getFilePart(result);
        load->source = soundwoofer::USER_SRC_ABSOLUTE;
        mCab->loadIr(load);
      }
      else {
        WDBGMSG("No file selected.\n");
      }
      // This is needed so the button which opened the pop up doesn't trigger the dialog again
      GetUI()->ReleaseMouseCapture();
    }

    /**
     * File drop is only supported in the standalone app
     */
    void OnDrop(const char* str) override {
      soundwoofer::SWImpulseShared load(new soundwoofer::SWImpulse());
      load->file = str;
      load->name = File::getFilePart(load->file);
      load->source = soundwoofer::USER_SRC_ABSOLUTE;
      mCab->loadIr(load);
      if (soundwoofer::file::isWaveName(load->name)) {
        mCab->loadIr(load);
      }
    }

    void Draw(IGraphics& g) override {
      NodeUi::Draw(g);
      g.DrawText(mBlocksizeText, mCab->mLoadedIr->name.c_str(), mRECT.GetVShifted(20));
    }

    void OnDetached() override {
      NodeUi::OnDetached();
      GetUI()->RemoveControl(mBrowseButton);
    }
  };

  GUITARD_REGISTER_NODE_UI(SimpleCabNode, SimpleCabNodeUi)
}
#endif
