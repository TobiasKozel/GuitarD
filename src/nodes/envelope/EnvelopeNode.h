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
      this->mPickerMode = true;
      MessageBus::fireEvent<Node*>(
        mBus, MessageBus::PickAutomationTarget, mParentNode
      );
    });
    mElements.Add(mPicker);
    mGraphics->AttachControl(mPicker);
  }

  void cleanUp() const override {
    NodeUi::cleanUp();
    mGraphics->RemoveControl(mPicker, true);
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
      mBus, MessageBus::VisualizeAutomationTargets, mParentNode
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
    // Maybe do some nice indication that this is the source automation
    //if (mPickerMode) {
    //  g.FillRect(IColor(80, 255, 0, 0), mTargetRECT);
    //}
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
  double avg = 0;
public:
  explicit EnvelopeNode(const std::string pType) {
    mType = pType;
  }

  ~EnvelopeNode() {
    for(int i = 0; i < mAutomationTargets.GetSize(); i++) {
      removeAutomationTarget(mAutomationTargets.Get(i));
    }
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
      "Filter", &filter, 0, 0, 1, 0.01
    );
    p->type = ParameterCoupling::Frequency;
    p->x = 0;
    p->y = -100;
    mParameters.Add(p);

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
    sample** buffer = mSocketsIn.Get(0)->mConnectedTo->mParentBuffer;
    mParameters.Get(1)->update();
    mParameters.Get(2)->update();
    double value = 0;
    for (int i = 0; i < nFrames; i++) {
      value += abs(buffer[0][i]);
    }
    value /= nFrames;
    avg = (filter * value + (1 - filter) * avg);

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
