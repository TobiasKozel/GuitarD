#pragma once
#include "../../node/Node.h"
namespace guitard {
  /**
   * This will take a signal and allow internal modulation for any other parameters
   */
  class EnvelopeNode final : public Node {
    PointerList<ParameterCoupling> mAutomationTargets;
    int mAutomationTargetCount = 0;
    sample gain = 0;
    sample filter = 0;
    sample offset = 0;
    sample avg = 0;
    sample current = 0;
  public:
    explicit EnvelopeNode(NodeList::NodeInfo* info) {
      mInfo = info;
      addMeter("Value", &avg, 0, 1);
    }

    void setup(const int pSamplerate, int pMaxBuffer, int, int, int) override {
      Node::setup(pSamplerate, pMaxBuffer, 1, 0, 2);
      mDimensions.y = 300;

      addByPassParam();
      const int top = -100;
      addParameter("Gain", &gain, 3.0, 0.0, 50.0, 0.01, { -80 , top });
      mParameters[
        addParameter("Filter", &filter, 0.9, 0, 1, 0.01, { 0 , top })
      ].type = ParameterCoupling::Frequency;
      addParameter("Offset", &offset, 0, -1, 1, 0.01, { 80 , top });
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
      if (!inputsReady() || mIsProcessed || byPass()) { return; }
      sample** buffer = mSocketsIn[0]->mConnectedTo[0]->mParentBuffer;
      mParameters[1].update();
      mParameters[2].update();
      mParameters[3].update();
      double value = 0;
      for (int i = 0; i < nFrames; i++) {
        value += abs(buffer[0][i]);
      }
      value /= nFrames;
      avg = (filter * value + (1 - filter) * avg);
      current = (avg + offset) * gain;
      for (int i = 0; i < mAutomationTargetCount; i++) {
        ParameterCoupling* c = mAutomationTargets.get(i);
        // scale them according to each of the max vals
        // TODOG take into account the scaling type e.g. frequency
        c->automation = current * c->max;
      }
      mIsProcessed = true;
    }
  };
  GUITARD_REGISTER_NODE(EnvelopeNode,
    "Envelope Automation Tool", "Automation",
    "Allows automating other parameters bades on the volume of the input signal"
  )
}


#ifndef GUITARD_HEADLESS
#include "../../ui/NodeUi.h"
namespace guitard {
  class EnvelopeNodeUi final : public NodeUi {
    IVButtonControl* mPicker = nullptr;
    bool mPickerMode = false;
    int mHistoryIndex = 0;
    const int mHistoryLength = PLUG_FPS * 2;
    double mHistory[PLUG_FPS * 2] = { 0 };
  public:
    EnvelopeNodeUi(Node* node, MessageBus::Bus* bus) : NodeUi(node, bus) { }

    void setUpControls() override {
      NodeUi::setUpControls();
      IRECT button{
        mTargetRECT.L + 30, mTargetRECT.T + 240,
        mTargetRECT.R - 30, mTargetRECT.B - 20
      };

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
      IControl::OnMouseDown(x, y, mod);
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

      mHistory[mHistoryIndex] = *(mNode->mMeters[0].value);

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
          ] * 5000 + 100,
          mHistoryLength - i + x,
              y - mHistory[index] * 5000 + 100,
              0, 2
              );
      }
      mDirty = true;
    }
  };
  GUITARD_REGISTER_NODE_UI(EnvelopeNode, EnvelopeNodeUi)
}
#endif
