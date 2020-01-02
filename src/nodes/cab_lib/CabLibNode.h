#pragma once
#include <thirdparty/json.hpp>
#include "src/node/Node.h"
#include "CabLibPopUp.h"


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
    mElements.Add(mEditButton);
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

  void cleanUp() override {
    GetUI()->RemoveControl(mEditButton, true);
    GetUI()->RemoveControl(mPopUp, true);
    NodeUi::cleanUp();
  }

  void registerSharedData(CabLibNodeSharedData* data) {
    mCabShared = data;
  }
};


class CabLibNode final : public Node {
  WrappedConvolver* mConvolver = nullptr;

  CabLibNodeSharedData mCabShared = { [&](IRBundle ir) {
    const int len = std::max(mCabShared.loadedIr.path.GetLength(), ir.path.GetLength());
    if (strncmp(mCabShared.loadedIr.path.Get(), ir.path.Get(), len) != 0) {
      // don't load if the Ir is the same;
      this->mCabShared.loadedIr = ir;
      this->mConvolver->resampleAndLoadIR(ir);
    }
}, IRBundle() };
public:
  CabLibNode(const std::string pType) {
    shared.type = pType;
    mStereo = 0;
    addByPassParam();
    addStereoParam();
    shared.parameters[1]->y = -30;
  }


  void serializeAdditional(nlohmann::json& serialized) override {
    serialized["path"] = mCabShared.loadedIr.path.Get();
  }
  void deserializeAdditional(nlohmann::json& serialized) override {
    try {
      IRBundle load;
      if (!serialized.contains("path")) {
        return;
      }
      const std::string path = serialized.at("path");
      load.path.Set(path.c_str());
      load.name.Set(load.path.get_filepart());
      mCabShared.loadedIr = load;
      mConvolver->resampleAndLoadIR(load);
    }
    catch (...) {
      WDBGMSG("Failed to load Cab node data!\n");
    }
  }

  void createBuffers() override {
    Node::createBuffers();
    mConvolver = new WrappedConvolver(mSampleRate, mMaxBuffer);
    mConvolver->resampleAndLoadIR(mCabShared.loadedIr);
  }

  void deleteBuffers() override {
    Node::deleteBuffers();
    delete mConvolver;
    mConvolver = nullptr;
  }

  virtual void OnSamplerateChanged(const int pSampleRate) {
    deleteBuffers();
    mSampleRate = pSampleRate;
    createBuffers();
  }

  void ProcessBlock(const int nFrames) override {
    if (!inputsReady() || mIsProcessed || byPass()) { return; }
    shared.parameters[1]->update();
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

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    shared.graphics = pGrahics;
    CabLibNodeUi* ui = new CabLibNodeUi(&shared);
    ui->registerSharedData(&mCabShared);
    mUi = ui;
    pGrahics->AttachControl(mUi);
    mUi->setColor(IColor(255, 150, 100, 100));
    mUi->setUp();
    mUiReady = true;
  }
};
