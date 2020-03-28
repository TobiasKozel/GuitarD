#pragma once
#include "IControl.h"
#include "../../bus/MessageBus.h"
#include "../../GUIConfig.h"
#include "../../../../thirdparty/soundwoofer/soundwoofer.h"

namespace guitard {
  class PresetEntryControl : public IControl {
    MessageBus::Bus* mBus = nullptr;
    soundwoofer::SWPresetShared mPreset = nullptr;
  public:
    PresetEntryControl(MessageBus::Bus* bus, soundwoofer::SWPresetShared preset) : IControl({}) {
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

      float y = mRECT.H() * 0.5 + mRECT.T;
      for (int i = 0; i < 5; i++) {
        float x = mRECT.R - (Theme::Preset::STAR_SIZE * Theme::Preset::STAR_PADDING) * 5
          + i * Theme::Preset::STAR_SIZE * Theme::Preset::STAR_PADDING;
        if (mPreset->rating <= i) {
          g.FillCircle(Theme::Preset::RATING_BG, x, y, Theme::Preset::STAR_SIZE);
        }
        else {
          g.FillCircle(Theme::Colors::ACCENT, x, y, Theme::Preset::STAR_SIZE);
        }
      }

      g.DrawText(Theme::Preset::TITLE, mPreset->name.c_str(), mRECT.GetHPadded(-10));

    }

    void OnMouseDown(const float x, const float y, const IMouseMod& mod) override {
      soundwoofer::preset::load(mPreset);
      if (!mPreset->data.empty()) {
        MessageBus::fireEvent<const char*>(
          mBus, MessageBus::LoadPresetFromString, mPreset->data.c_str()
        );
      }
    }
  };
}
