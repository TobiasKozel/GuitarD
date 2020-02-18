#pragma once

#include "../../node/Node.h"

namespace guitard {
#ifndef GUITARD_HEADLESS
  class LfoNodeUi final : public NodeUi {
    IVButtonControl* mPicker = nullptr;
    bool mPickerMode = false;
    int mHistoryIndex = 0;
    const int mHistoryLength = PLUG_FPS * 2;
    double mHistory[PLUG_FPS * 2] = { 0 };
  public:
    LfoNodeUi(NodeShared* param) : NodeUi(param) {
    }

    void setUpControls() override {
      NodeUi::setUpControls();
      IRECT button{ mTargetRECT.L + 30, mTargetRECT.T + 240, mTargetRECT.R - 30, mTargetRECT.B - 20 };
      mPicker = new IVButtonControl(button, [&](IControl* pCaller) {
        this->mPickerMode = true;
        MessageBus::fireEvent<Node*>(
          shared->bus, MessageBus::PickAutomationTarget, shared->node
        );
      }, "Pick Automation Target", DEFAULT_STYLE, true);
      mElements.add(mPicker);
      shared->graphics->AttachControl(mPicker);
    }

    void OnDetached() override {
      shared->graphics->RemoveControl(mPicker);
      NodeUi::OnDetached();
    }

    void OnMouseDown(const float x, const float y, const IMouseMod& mod) override {
      NodeUi::OnMouseDown(x, y, mod);
      if (mPickerMode) {
        mPickerMode = false;
      }
      mDirty = true;
    }

    void OnMouseOver(float x, float y, const IMouseMod& mod) override {
      mMouseIsOver = true;
      MessageBus::fireEvent<Node*>(
        shared->bus, MessageBus::VisualizeAutomationTargets, shared->node
      );
    }

    void OnMouseOut() override {
      mMouseIsOver = false;
      MessageBus::fireEvent<Node*>(
        shared->bus, MessageBus::VisualizeAutomationTargets, nullptr
      );
    }

    void Draw(IGraphics& g) override {
      NodeUi::Draw(g);
      if (!mDoRender) { return; }
      mHistoryIndex++;
      if (mHistoryIndex >= mHistoryLength) {
        mHistoryIndex = 0;
      }

      mHistory[mHistoryIndex] = *(shared->meters[0]->value) * 50;

      float x = mTargetRECT.L + 40;
      float y = mTargetRECT.T + 100;
      g.FillRect(iplug::igraphics::COLOR_GRAY, IRECT(x, y, x + mHistoryLength, y + 100));
      const int stride = 1;
      for (int i = 0; i < mHistoryLength; i += stride) {
        int index = mHistoryIndex - i;
        if (index < 0) {
          index += mHistoryLength;
        }
        g.DrawLine(
          iplug::igraphics::COLOR_WHITE,
          mHistoryLength - i - stride + x,
          y - mHistory[
              (index - stride < 0) ? index - stride + mHistoryLength :
              index - stride
          ] + 50,
          mHistoryLength - i + x,
              y - mHistory[index] + 50,
              0, 2
        );
      }
      mDirty = true;
    }

  };
#endif


  class LfoNode : public Node {
    PointerList<ParameterCoupling> mAutomationTargets;
    int mAutomationTargetCount = 0;
    sample mTime = 0;
    sample mLfoVal = 0;
    sample mLfoF = 0.1; // In Hz
    sample mNoise = 0;
    sample mGain = 1.0;
  public:
    explicit LfoNode(NodeList::NodeInfo* info) {
      shared.info = info;
      shared.meters[shared.meterCount] = new MeterCoupling{ &mLfoVal, "Value", 0, 1 };
      shared.meterCount++;
    }

    void setup(MessageBus::Bus* pBus, const int pSamplerate = 48000, int pMaxBuffer = MAX_BUFFER, int pChannles = 2, int pInputs = 1, int pOutputs = 1) override {
      Node::setup(pBus, pSamplerate, pMaxBuffer, 2, 0, 0); // No output sockets
      addByPassParam();

      shared.parameters[shared.parameterCount] = ParameterCoupling(
        "Frequency", &mLfoF, 0.2, 0.001, 20.0, 0.01
      );

      shared.height = 300;
      int top = -100;
      shared.parameters[shared.parameterCount].type = ParameterCoupling::Frequency;
      shared.parameters[shared.parameterCount].x = -80;
      shared.parameters[shared.parameterCount].y = top;
      shared.parameterCount++;

      shared.parameters[shared.parameterCount] = ParameterCoupling(
        "Noise", &mNoise, 0, 0, 1, 0.001
      );
      shared.parameters[shared.parameterCount].x = 0;
      shared.parameters[shared.parameterCount].y = top;
      shared.parameterCount++;

      shared.parameters[shared.parameterCount] = ParameterCoupling(
        "Gain", &mGain, 1, -1, 1, 0.01
      );
      shared.parameters[shared.parameterCount].x = 80;
      shared.parameters[shared.parameterCount].y = top;
      shared.parameterCount++;
    }

    void cleanUp() override {
      Node::cleanUp();
      for (int i = 0; i < mAutomationTargets.size(); i++) {
        removeAutomationTarget(mAutomationTargets[i]);
      }
    }

    void addAutomationTarget(ParameterCoupling* c) override {
      // Check if it's our own target
      if (mAutomationTargets.find(c) == -1) {
        if (c->automationDependency != nullptr) {
          // If not, but there's still a target, get rid of it
          c->automationDependency->removeAutomationTarget(c);
        }
        mAutomationTargets.add(c);
        mAutomationTargetCount++;
        if (c->automationDependency != nullptr) {
          WDBGMSG("Trying to attach automation to a Param with an automation!\n");
          assert(false);
        }
        c->automationDependency = this;
      }
      else {
        // It's ours so get rid of it
        removeAutomationTarget(c);
      }
    }

    void removeAutomationTarget(ParameterCoupling* c) override {
      const int i = mAutomationTargets.find(c);
      if (i != -1) {
        mAutomationTargets.remove(i);
        mAutomationTargetCount--;
        c->automationDependency = nullptr;
        c->automation = 0;
      }
    }

    void ProcessBlock(const int nFrames) override {
      if (mIsProcessed || byPass()) { return; }
      shared.parameters[1].update();
      shared.parameters[2].update();
      shared.parameters[3].update();
      mTime += (static_cast<sample>(nFrames) / static_cast<sample>(mSampleRate)) * mLfoF * 2.0 * PI;

      mLfoVal = sin(mTime);
      mLfoVal += sin(mTime * 1000.0) * mNoise;
      mLfoVal *= mGain;
      for (int i = 0; i < mAutomationTargetCount; i++) {
        ParameterCoupling* c = mAutomationTargets.get(i);
        // scale them according to each of the max vals
        // TODOG take into account the scaling type e.g. frequency
        c->automation = mLfoVal * c->max;
      }
      mIsProcessed = true;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      shared.graphics = pGrahics;
      mUi = new LfoNodeUi(&shared);
      pGrahics->AttachControl(mUi);
      mUi->setColor(Theme::Categories::AUTOMATION);
      mUi->setUp();
      mUiReady = true;
    }
#endif
  };
}
