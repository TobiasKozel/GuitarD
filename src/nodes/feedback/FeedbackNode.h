#pragma once
#include "src/node/Node.h"
#include "circbuf.h"

namespace guitard {
#ifndef GUITARD_HEADLESS
  class FeedbackNodeUi : public NodeUi {
    IVButtonControl* mBrowseButton = nullptr;
    MessageBus::Subscription<BlockSizeEvent*> mMaxBlockSizeEvent;
    IPopupMenu mMenu{ "Delay", {"1", "8", "16", "32", "64", "128", "256", "512"},
      [&](IPopupMenu* pMenu) {
      IPopupMenu::Item* itemChosen = pMenu->GetChosenItem();
      if (itemChosen) {
        const char* text = itemChosen->GetText();
        const int size = std::stoi(text);
        BlockSizeEvent b = {size, true};
        MessageBus::fireEvent(shared->bus, MessageBus::MaxBlockSizeEvent, &b);
      }
    } };
  public:
    FeedbackNodeUi(NodeShared* param) : NodeUi(param) {
      mMaxBlockSizeEvent.subscribe(param->bus, MessageBus::MaxBlockSizeEvent, [&](BlockSizeEvent* e) {
        if (e->set) {
          std::string label = std::to_string(shared->maxBlockSize) + " Samples";
          this->mBrowseButton->SetLabelStr(label.c_str());
          // refresh
        }
      });
    }

    void setUpControls() override {
      NodeUi::setUpControls();
      std::string label = std::to_string(shared->maxBlockSize) + " Samples";
      const IRECT button{ mTargetRECT.L + 50, mTargetRECT.T + 130, mTargetRECT.R - 50, mTargetRECT.B - 20 };
      mBrowseButton = new IVButtonControl(button, [&](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        float x, y;
        this->shared->graphics->GetMouseDownPoint(x, y);
        this->shared->graphics->CreatePopupMenu(*pCaller, mMenu, x, y);

      }, label.c_str(), DEFAULT_STYLE, true, false);
      mElements.add(mBrowseButton);
      shared->graphics->AttachControl(mBrowseButton);
    }

    void OnDetached() override {
      NodeUi::OnDetached();
      shared->graphics->RemoveControl(mBrowseButton);
    }

  };
#endif

  class FeedbackNode final : public Node {
    sample gain = 0.0;
    WDL_TypedCircBuf<sample> mPrevL;
    WDL_TypedCircBuf<sample> mPrevR;

  public:
    FeedbackNode(const std::string pType) {
      shared.type = pType;
    }

    void setup(MessageBus::Bus* pBus, int pSamplerate, int pMaxBuffer, int pChannels = 2, int pInputs = 1, int pOutputs = 1) override {
      Node::setup(pBus, pSamplerate, pMaxBuffer, pChannels, pInputs, pOutputs);
      addByPassParam();
      shared.parameters[shared.parameterCount] = ParameterCoupling(
        "Gain", &gain, -40, -130.0, 40.0, 0.1
      );
      shared.parameters[shared.parameterCount].type = ParameterCoupling::Gain;
      shared.parameterCount++;



      shared.socketsIn[0]->mX += shared.width - 30;
      //shared.socketsIn[0]->mY = 50;
      shared.socketsOut[0]->mX -= shared.width - 30;
      //shared.socketsOut[0]->mY = 50;

    }

    void createBuffers() override {
      Node::createBuffers();
      mPrevL.SetSize(shared.maxBlockSize + 1); // This is needed or the ring buffer will
      mPrevR.SetSize(shared.maxBlockSize + 1); // underflow for some reason
    }

    void deleteBuffers() override {
      Node::deleteBuffers();
      mPrevL.SetSize(0);
      mPrevR.SetSize(0);
    }


    void ProcessBlock(int nFrames) override {
      if (byPass()) { return; }
      if (mIsProcessed == false) {
        shared.parameters[1].update();
        sample val = ParameterCoupling::dbToLinear(gain);
        if (mPrevL.NbInBuf() > shared.maxBlockSize) { // nFrames can never been larger than mMaxBuffer
          mPrevL.Get(mBuffersOut[0][0], nFrames);
          mPrevR.Get(mBuffersOut[0][1], nFrames);
          for (int i = 0; i < nFrames; i++) {
            mBuffersOut[0][0][i] *= val;
            mBuffersOut[0][1][i] *= val;
          }
          mIsProcessed = true;
        }
        else {
          outputSilence();
        }
      }

      if (inputsReady()) {
        sample** buffer = shared.socketsIn[0]->mConnectedTo[0]->mParentBuffer;
        mPrevL.Add(buffer[0], nFrames);
        mPrevR.Add(buffer[1], nFrames);
      }
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      shared.graphics = pGrahics;
      mUi = new FeedbackNodeUi(&shared);
      mUi->setColor(Theme::Categories::TOOLS);
      pGrahics->AttachControl(mUi);
      mUi->setUp();
      mUiReady = true;
    }
#endif
  };
}