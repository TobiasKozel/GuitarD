#pragma once
#include "src/node/Node.h"
#include "circbuf.h"

class EnvelopeNodeUi final : public NodeUi {
  IVButtonControl* mPicker = nullptr;
  bool mPickerMode = false;
  int mHistoryIndex = 0;
  const int mHistoryLength = PLUG_FPS * 2;
  double mHistory[PLUG_FPS * 2] = { 0 };
public:
  EnvelopeNodeUi(NodeShared* param) : NodeUi(param) {
  }

  void setUpControls() override {
    NodeUi::setUpControls();
    IRECT button{ mTargetRECT.L + 30, mTargetRECT.T + 240, mTargetRECT.R - 30, mTargetRECT.B -20};
    mPicker = new IVButtonControl(button, [&](IControl* pCaller) {
      this->mPickerMode = true;
      MessageBus::fireEvent<Node*>(
        shared->bus, MessageBus::PickAutomationTarget, shared->node
      );
    }, "Pick Automation Target", DEFAULT_STYLE, true);
    mElements.Add(mPicker);
    shared->graphics->AttachControl(mPicker);
  }

  void cleanUp() override {
    shared->graphics->RemoveControl(mPicker, true);
    NodeUi::cleanUp();
  }

  void OnMouseDown(const float x, const float y, const IMouseMod& mod) override {
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

    mHistory[mHistoryIndex] = *(shared->meters[0]->value);

    float x = mTargetRECT.L + 40;
    float y = mTargetRECT.T + 100;
    g.FillRect(COLOR_GRAY, IRECT(x, y, x + mHistoryLength, y + 100));
    const int stride = 1;
    for (int i = 0; i < mHistoryLength; i += stride) {
      int index = mHistoryIndex - i;
      if (index < 0) {
        index += mHistoryLength;
      }
      g.DrawLine(
        COLOR_WHITE,
        mHistoryLength - i - stride + x,
        y - mHistory[
          (index - stride < 0) ? index - stride + mHistoryLength :
          index - stride
        ] * 5000 + 100,
        mHistoryLength - i + x,
        y -mHistory[index] * 5000 + 100,
        0, 2
      );
    }
    mDirty = true;
  }

};

/**
 * This will take a signal and allow internal modulation for any other parameters
 */
class EnvelopeNode final : public Node {
  WDL_PtrList<ParameterCoupling> mAutomationTargets;
  int mAutomationTargetCount = 0;
  double gain = 0;
  double filter = 0;
  double offset = 0;
  double avg = 0;
  double current = 0;
public:
  explicit EnvelopeNode(const std::string pType) {
    shared.type = pType;
    shared.meters[shared.meterCount] = new MeterCoupling{ &avg, "Value", 0, 1 };
    shared.meterCount++;
  }

  void setup(MessageBus::Bus* pBus, const int pSamplerate = 48000, int pMaxBuffer = MAX_BUFFER, int pChannles = 2, int pInputs = 1, int pOutputs = 1) override {
    Node::setup(pBus, pSamplerate, pMaxBuffer, 2, 1, 0);
    addByPassParam();

    ParameterCoupling* p = new ParameterCoupling(
      "Gain", &gain, 0.0, 0.0, 50.0, 0.01
    );

    shared.height = 300;
    const int top = -100;
    p->x = -80;
    p->y = top;
    shared.parameters[shared.parameterCount] = p;
    shared.parameterCount++;

    p = new ParameterCoupling(
      "Filter", &filter, 0, 0, 1, 0.01
    );
    p->type = ParameterCoupling::Frequency;
    p->x = 0;
    p->y = top;
    shared.parameters[shared.parameterCount] = p;
    shared.parameterCount++;

    p = new ParameterCoupling(
      "Offset", &offset, 0, -1, 1, 0.01
    );
    p->x = 80;
    p->y = top;
    shared.parameters[shared.parameterCount] = p;
    shared.parameterCount++;

  }

  void cleanUp() override {
    Node::cleanUp();
    for (int i = 0; i < mAutomationTargets.GetSize(); i++) {
      removeAutomationTarget(mAutomationTargets.Get(i));
    }
  }

  void addAutomationTarget(ParameterCoupling* c) override {
    // Check if it's our own target
    if (mAutomationTargets.Find(c) == -1) {
      if (c->automationDependency != nullptr) {
        // If not, but there's still a target, get rid of it
        c->automationDependency->removeAutomationTarget(c);
      }
      mAutomationTargets.Add(c);
      mAutomationTargetCount++;
      if (c->automationDependency != nullptr) {
        WDBGMSG("Trying to attach automation to a Param with an automation!\n");
        assert(true);
      }
      c->automationDependency = this;
    }
    else {
      // It's ours so get rid of it
      removeAutomationTarget(c);
    }
  }

  void removeAutomationTarget(ParameterCoupling* c) override {
    const int i = mAutomationTargets.Find(c);
    if (i != -1) {
      mAutomationTargets.Delete(i);
      mAutomationTargetCount--;
      c->automationDependency = nullptr;
      c->automation = 0;
    }
  }

  void ProcessBlock(const int nFrames) override {
    if (!inputsReady() || mIsProcessed || byPass()) { return; }
    sample** buffer = shared.socketsIn[0]->mConnectedTo[0]->mParentBuffer;
    shared.parameters[1]->update();
    shared.parameters[2]->update();
    shared.parameters[3]->update();
    double value = 0;
    for (int i = 0; i < nFrames; i++) {
      value += abs(buffer[0][i]);
    }
    value /= nFrames;
    avg = (filter * value + (1 - filter) * avg);
    current = (avg + offset) * gain;
    for (int i = 0; i < mAutomationTargetCount; i++) {
      ParameterCoupling* c = mAutomationTargets.Get(i);
      // scale them according to each of the max vals
      // TODOG take into account the scaling type e.g. frequency
      c->automation = current * c->max;
    }
    mIsProcessed = true;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    shared.graphics = pGrahics;
    mUi = new EnvelopeNodeUi(&shared);
    pGrahics->AttachControl(mUi);
    mUi->setColor(Theme::Categories::AUTOMATION);
    mUi->setUp();
    mUiReady = true;
  }
};
