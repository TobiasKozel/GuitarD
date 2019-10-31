#pragma once

#include "src/node/Node.h"
#include "src/node/NodeUi.h"

#define TWO_YROOT_TWELVE 1.05946309435929526456

class InputNodeUi : public NodeUi {
  map<float, string> equalPitches;
  const char* mNotes[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
  bool mEnableTuner;
  const char* mCurrentQuantizedPitch;
  int mTunerCentsOff;
  float mCurrentFrequency;
  void update() {
  }
public:
  InputNodeUi(NodeUiParam param) : NodeUi(param) {
    mCurrentFrequency = 440.f;
    mCurrentQuantizedPitch = mNotes[0];
    mTunerCentsOff = 0;
    mEnableTuner = true;
    float freq = 16.35159783f;
    for (int i = 0; i < 108; i++) {
      equalPitches.insert(pair<float, const char*>(freq, mNotes[i % 12]));
      freq *= TWO_YROOT_TWELVE;
    }
  }

  void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    if (mDragging) {
      mDragging = false;
      return;
    }
    if (mDisconnectAllButton.Contains(IRECT(x, y, x, y))) {
      MessageBus::fireEvent<Node*>("NodeDisconnectAll", mParentNode);
    }
  }

  void Draw(IGraphics& g) override {
    g.DrawBitmap(mBitmap, mRECT, 1, &mBlend);
    g.DrawRect(IColor(255, 0, 255, 0), mDisconnectAllButton);
    if (mEnableTuner) {
      g.DrawRect(IColor(255, 0, 255, 0), mDisconnectAllButton);
      mDirty = true;
    }
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    mDragging = true;
    translate(dX, dY);
  }
};

class InputNode : public Node {
public:
  InputNode(int channels) : Node() {
    setup(0, MAXBUFFER, channels, 0, 1);
  }

  void ProcessBlock(int) {}

  void CopyIn(iplug::sample** in, int nFrames) {
    for (int c = 0; c < channelCount; c++) {
      for (int i = 0; i < nFrames; i++) {
        outputs[0][c][i] = in[c][i];
      }
    }
    isProcessed = true;
  }

  void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
    mUi = new InputNodeUi(NodeUiParam {
      pGrahics,
      PNGGENERICBG_FN,
      &X, &Y, nullptr,
      &parameters,
      &inSockets,
      &outSockets,
      this
    });
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    uiReady = true;
  }
};