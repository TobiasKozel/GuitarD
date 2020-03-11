#pragma once

#include "../../node/Node.h"

namespace guitard {
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
      mInfo = info;

    }

    void setup(const int pSamplerate, int pMaxBuffer, int, int, int) override {
      Node::setup(pSamplerate, pMaxBuffer, 0, 0, 2); // No output sockets
      mDimensions.y = 300;
      addMeter("Value", &mLfoVal, 0, 1);
      float top = -100;
      addByPassParam();
      mParameters[
        addParameter("Frequency", &mLfoF, 0.2, 0.001, 20.0, 0.01, { -80, top })
      ].type = ParameterCoupling::Frequency;
      addParameter("Noise", &mNoise, 0, 0, 1, 0.001, { 0, top });
      addParameter("Gain", &mGain, 1, -2, 2, 0.001, { 80, top });
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
      if (mIsProcessed) { return; }
      mParameters[0].update();
      if (mByPassed > 0.5) {
        for (int i = 0; i < mAutomationTargetCount; i++) {
          ParameterCoupling* c = mAutomationTargets.get(i);
          c->automation = 0.0;
        }
        mIsProcessed = true;
        return;
      }
      mParameters[1].update();
      mParameters[2].update();
      mParameters[3].update();
      mTime += (static_cast<sample>(nFrames) / static_cast<sample>(mSampleRate))* mLfoF * 2.0 * PI;

      mLfoVal = sin(mTime) * 0.5 + 0.5;
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
  };
  GUITARD_REGISTER_NODE(
    LfoNode, "LFO Automation Tool", "Automation",
    "Lfo to automate other parameters", "image"
  )
}


#ifndef GUITARD_HEADLESS
#include "../../ui/NodeUi.h"
namespace guitard {
  class LfoNodeUi final : public guitard::NodeUi {
    IVButtonControl* mPicker = nullptr;
    bool mPickerMode = false;
    int mHistoryIndex = 0;
    static const int mHistoryLength = PLUG_FPS * 2;
    double mHistory[mHistoryLength] = { 0 };
  public:
    LfoNodeUi(Node* node, MessageBus::Bus* bus) : NodeUi(node, bus) { }

    void setUpControls() override {
      NodeUi::setUpControls();
      IRECT button{ mTargetRECT.L + 30, mTargetRECT.T + 240, mTargetRECT.R - 30, mTargetRECT.B - 20 };
      mPicker = new IVButtonControl(button, [&](IControl* pCaller) {
        this->mPickerMode = true;
        MessageBus::fireEvent<Node*>(
          mBus, MessageBus::PickAutomationTarget, mNode
        );
      }, "Pick Automation Target", DEFAULT_STYLE, true);
      mElements.add(mPicker);
      GetUI()->AttachControl(mPicker);
    }

    void OnDetached() override {
      GetUI()->RemoveControl(mPicker);
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
        mBus, MessageBus::VisualizeAutomationTargets, mNode
      );
    }

    void OnMouseOut() override {
      mMouseIsOver = false;
      MessageBus::fireEvent<Node*>(
        mBus, MessageBus::VisualizeAutomationTargets, nullptr
      );
    }

    void Draw(IGraphics& g) override {
      NodeUi::Draw(g);
      if (!mDoRender) { return; }
      mHistoryIndex++;
      if (mHistoryIndex >= mHistoryLength) {
        mHistoryIndex = 0;
      }

      mHistory[mHistoryIndex] = *(mNode->mMeters[0].value) * 25;

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

  GUITARD_REGISTER_NODE_UI(LfoNode, LfoNodeUi)
}
#endif
