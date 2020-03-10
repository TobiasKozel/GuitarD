#pragma once
#include "../../node/Node.h"
#include "../../types/ringBuffer.h"

namespace guitard {
  class FeedbackNode final : public Node {
    sample gain = 0.0;
    RingBuffer<sample> mPrevL;
    RingBuffer<sample> mPrevR;

  public:
    FeedbackNode(NodeList::NodeInfo* info) {
      mInfo = info;
    }

    void setup(int pSamplerate, int pMaxBuffer, int pInputs = 1, int pOutputs = 1, int pChannels = 2) override {
      Node::setup(pSamplerate, pMaxBuffer, pInputs, pOutputs, pChannels);
      addByPassParam();
      mParameters[
        addParameter("Gain", &gain, -40, -130.0, 40.0, 0.1)
      ].type = ParameterCoupling::Gain;

      // Swap the socket positions
      mSocketsIn[0]->mRel.x = mDimensions.x * 0.5 - 20;
      mSocketsOut[0]->mRel.x = -mDimensions.x * 0.5 + 20;

    }

    void createBuffers() override {
      Node::createBuffers();
      mPrevL.setSize(mMaxBlockSize + 1); // This is needed or the ring buffer will
      mPrevR.setSize(mMaxBlockSize + 1); // underflow for some reason
    }

    void deleteBuffers() override {
      Node::deleteBuffers();
      mPrevL.setSize(0);
      mPrevR.setSize(0);
    }


    void ProcessBlock(int nFrames) override {
      if (byPass()) { return; }
      if (mIsProcessed == false) {
        if (mPrevL.inBuffer() > mMaxBlockSize&& mPrevL.nFree() == 0) { // nFrames can never been larger than mMaxBuffer
          mParameters[1].update();
          sample val = ParameterCoupling::dbToLinear(gain);
          mPrevL.get(mBuffersOut[0][0], nFrames);
          mPrevR.get(mBuffersOut[0][1], nFrames);
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
        sample** buffer = mSocketsIn[0]->mConnectedTo[0]->mParentBuffer;
        mPrevL.add(buffer[0], nFrames);
        mPrevR.add(buffer[1], nFrames);
      }
    }
  };

  GUITARD_REGISTER_NODE(FeedbackNode,
    "Feedback", "Signal Flow", "Allows feeding the signal backwards, be careful!", "image"
  )

#ifndef GUITARD_HEADLESS
  class FeedbackNodeUi : public NodeUi {
    IVButtonControl* mBrowseButton = nullptr;
    MessageBus::Subscription<BlockSizeEvent*> mMaxBlockSizeEvent;
    IPopupMenu mMenu{ "Delay", {"1", "2", "4", "8", "16", "32", "64", "128", "256", "512"},
      [&](IPopupMenu* pMenu) {
      IPopupMenu::Item* itemChosen = pMenu->GetChosenItem();
      if (itemChosen) {
        const char* text = itemChosen->GetText();
        const int size = std::stoi(text);
        BlockSizeEvent b = {size, true};
        MessageBus::fireEvent(mBus, MessageBus::MaxBlockSizeEvent, &b);
      }
    } };
  public:
    FeedbackNodeUi(Node* node, MessageBus::Bus* bus) : NodeUi(node, bus) {
      mMaxBlockSizeEvent.subscribe(mBus, MessageBus::MaxBlockSizeEvent, [&](BlockSizeEvent* e) {
        if (e->set) {
          String label = std::to_string(mNode->mMaxBlockSize) + " Samples";
          this->mBrowseButton->SetLabelStr(label.c_str());
        }
      });
    }

    void setUpControls() override {
      NodeUi::setUpControls();
      std::string label = std::to_string(mNode->mMaxBlockSize) + " Samples";
      const IRECT button{ mTargetRECT.L + 50, mTargetRECT.T + 130, mTargetRECT.R - 50, mTargetRECT.B - 20 };
      mBrowseButton = new IVButtonControl(button, [&](IControl* pCaller) {
        SplashClickActionFunc(pCaller);
        float x, y;
        GetUI()->GetMouseDownPoint(x, y);
        GetUI()->CreatePopupMenu(*pCaller, mMenu, x, y);

      }, label.c_str(), DEFAULT_STYLE, true, false);
      mElements.add(mBrowseButton);
      GetUI()->AttachControl(mBrowseButton);
    }

    void OnDetached() override {
      NodeUi::OnDetached();
      GetUI()->RemoveControl(mBrowseButton);
    }
  };

  GUITARD_REGISTER_NODE_UI(FeedbackNode, FeedbackNodeUi)
#endif
}