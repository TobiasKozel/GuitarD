#pragma once
#include "IControl.h"
#include "src/misc/MessageBus.h"
#include "src/ui/theme.h"
#include "thirdparty/soundwoofer.h"

using namespace iplug;
using namespace igraphics;

class PresetEntryControl : public IControl {
  MessageBus::Bus* mBus = nullptr;
  SoundWoofer::SWPreset* mPreset = nullptr;
public:
  PresetEntryControl(MessageBus::Bus* bus, SoundWoofer::SWPreset* preset) : IControl({}) {
    mBus = bus;
    mPreset = preset;
  }

  void OnResize() override {
    mRECT.T = 0;
    mRECT.B = 128;
    mTargetRECT = mRECT;
  }

  void Draw(IGraphics& g) override {
    g.FillRect(Theme::Colors::ACCENT, mRECT);
    g.DrawText(Theme::Gallery::CATEGORY_TITLE, mPreset->name.c_str(), mRECT);
  }

  void OnMouseUp(const float x, const float y, const IMouseMod& mod) override {
    if (mod.L) {
      // MessageBus::fireEvent<NodeList::NodeInfo>(mBus, MessageBus::NodeAdd, elem->mInfo);
    }
  }
};