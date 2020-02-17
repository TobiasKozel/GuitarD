#pragma once
#include "../../node/Node.h"
#include "../../types/types.h"
#include <map>

#define TWO_YROOT_TWELVE 1.05946309435929526456f

namespace guitard {
#ifndef GUITARD_HEADLESS
  class TunerNodeUi : public NodeUi {
    std::map<float, String> equalPitches;
    const char* mNotes[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    const char* mCurrentQuantizedPitch;
    int mTunerCentsOff;
    float mCurrentFrequency;
    iplug::igraphics::IText mText = DEBUG_FONT;
  public:
    TunerNodeUi(NodeShared* param) : NodeUi(param) {
      float freq = 16.35159783f;
      for (int i = 0; i < 108; i++) {
        equalPitches.insert(std::pair<float, const char*>(freq, mNotes[i % 12]));
        freq *= TWO_YROOT_TWELVE;
      }
    }
    void Draw(IGraphics& g) override {
      NodeUi::Draw(g);
      IRECT m = mRECT.GetPadded(-100);
      g.FillRect(
        IColor(255, 255, 50, 50), m
      );
      mDirty = true;
    }
  };
#endif

  class TunerNode final : public Node {
  public:
    TunerNode(const NodeList::NodeInfo* info) {
      shared.info = info;
    }

#ifndef GUITARD_HEADLESS
    void setupUi(iplug::igraphics::IGraphics* pGrahics) override {
      shared.graphics = pGrahics;
      mUi = new TunerNodeUi(&shared);
      mUi->setColor(Theme::Categories::DISTORTION);
      pGrahics->AttachControl(mUi);
      mUi->setUp();
      mUiReady = true;
    }
#endif
  };
}