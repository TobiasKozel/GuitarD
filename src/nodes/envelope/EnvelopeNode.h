#pragma once
#include "src/node/Node.h"

class EnvelopeNodeUi final : public NodeUi {
  IVButtonControl* mPicker = nullptr;
  bool mPickerMode = false;
public:
  EnvelopeNodeUi(const NodeUiParam param) : NodeUi(param) {
  }

  void setUpControls() override {
    NodeUi::setUpControls();
    mPicker = new IVButtonControl(mRECT.GetPadded(-100), [&](IControl* pCaller) {
      this->enterPickerMode();
    });
    mElements.Add(mPicker);
    mGraphics->AttachControl(mPicker);
  }

  void cleanUp() const override {
    NodeUi::cleanUp();
    mGraphics->RemoveControl(mPicker, true);
  }

  void enterPickerMode() {
    mNoScale = true;
    SetTargetRECT(mGraphics->GetBounds());
    mDirty = true;
    mPickerMode = true;
  }

  void OnMouseDown(float x, float y, const IMouseMod& mod) override {
    if (mPickerMode) {
      SetTargetRECT(mRECT);
      mPickerMode = false;
      IControl* target = mGraphics->GetControl(
        mGraphics->GetMouseControlIdx(x, y, false)
      );
      if (target != nullptr) {
        MessageBus::fireEvent<AutomationAttachRequest>(
          mBus, MessageBus::AttachAutomation,
          AutomationAttachRequest{
            mParentNode, target
          }
        );
      }
    }
    mDirty = true;
  }

  void Draw(IGraphics& g) override {
    NodeUi::Draw(g);
    if (mPickerMode) {
      g.FillRect(IColor(80, 255, 0, 0), mTargetRECT);
    }
  }

};

class EnvelopeNode final : public Node {
  WDL_PtrList<ParameterCoupling> mAutomationTargets;
  int mAutomationTargetCount = 0;
  double gain = 0;
  double filter = 0;
  double avg = 0;
public:
  explicit EnvelopeNode(const std::string pType) : Node() {
    mType = pType;
  }

  void setup(MessageBus::Bus* pBus, const int pSamplerate = 48000, int pMaxBuffer = MAX_BUFFER, int pChannles = 2, int pInputs = 1, int pOutputs = 1) override {
    Node::setup(pBus, pSamplerate, pMaxBuffer, 2, 1, 0);
    addByPassParam();

    ParameterCoupling* p = new ParameterCoupling(
      "Gain", &gain, 0.0, 0.0, 50.0, 0.01
    );
    p->x = -100;
    p->y = -100;
    mParameters.Add(p);

    p = new ParameterCoupling(
      "Filter", &filter, 0, 0, 1.0, 0.01
    );
    p->x = 0;
    p->y = -100;
    mParameters.Add(p);

  }

  void addAutomationTarget(ParameterCoupling* c) override {
    if (mAutomationTargets.Find(c) == -1) {
      mAutomationTargets.Add(c);
      mAutomationTargetCount++;
    }
  }

  void removeAutomationTarget(ParameterCoupling* c) override {
    int i = mAutomationTargets.Find(c);
    if (i != -1) {
      mAutomationTargets.Delete(i);
      mAutomationTargetCount--;
    }
  }

  void ProcessBlock(int nFrames) override {
    if (!inputsReady() || mIsProcessed || byPass()) { return; }
    sample** buffer = mSocketsIn.Get(0)->mConnectedTo->mParentBuffer;
    mParameters.Get(1)->update();
    mParameters.Get(2)->update();
    double value = 0;
    for (int i = 0; i < nFrames; i++) {
      value += buffer[0][i];
    }
    value /= nFrames;
    avg = (filter * avg + (1 - filter) * value);
    for (int i = 0; i < mAutomationTargetCount; i++) {
      ParameterCoupling* c = mAutomationTargets.Get(i);
      c->automation = avg * gain * c->max;
    }
    mIsProcessed = true;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    mUi = new EnvelopeNodeUi(NodeUiParam{
      mBus, pGrahics, 300, 300, &mX, &mY,
      &mParameters, &mSocketsIn, &mSocketsOut, this
    });
    pGrahics->AttachControl(mUi);
    mUi->setColor(Theme::Categories::AUTOMATION);
    mUi->setUp();
    mUiReady = true;
  }
};
