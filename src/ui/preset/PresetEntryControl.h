#pragma once
#include "IControl.h"
#include "src/misc/MessageBus.h"
#include "src/ui/theme.h"
#include "soundwoofer/soundwoofer.h"

using namespace iplug;
using namespace igraphics;

class PresetEntryControl : public IControl {
  MessageBus::Bus* mBus = nullptr;
  SoundWoofer::SWPreset* mPreset = nullptr;
public:
  PresetEntryControl(MessageBus::Bus* bus, SoundWoofer::SWPreset* preset) : IControl({}) {
    mBus = bus;
    mPreset = preset;
    mRECT.T = 0;
    mRECT.B = 24;
    mTargetRECT = mRECT;
  }

  void OnResize() override {

  }

  void Draw(IGraphics& g) override {
    if (mMouseIsOver) {
      g.FillRect(Theme::Gallery::CATEGORY_TITLE_BG_HOVER, mRECT);
    }
    else {
      g.FillRect(Theme::Gallery::CATEGORY_TITLE_BG, mRECT);
    }
    
    g.DrawText(Theme::Gallery::CATEGORY_TITLE, mPreset->name.c_str(), mRECT);
  }

  void OnMouseUp(const float x, const float y, const IMouseMod& mod) override {
    if (mod.L) {
      MessageBus::fireEvent<const char*>(mBus, MessageBus::LoadPresetFromString, mPreset->data.c_str());
    }
  }
};