#pragma once
#include "IControl.h"
#include "../../misc/MessageBus.h"
#include "../../ui/theme.h"
#include "../../../thirdparty/soundwoofer/soundwoofer.h"

namespace guitard {
  class PresetEntryControl : public IControl {
    MessageBus::Bus* mBus = nullptr;
    soundwoofer::SWPresetsShared mPreset = nullptr;
  public:
    PresetEntryControl(MessageBus::Bus* bus, soundwoofer::SWPresetsShared preset) : IControl({}) {
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
      soundwoofer::preset::load(mPreset);
      if (!mPreset->data.empty()) {
        MessageBus::fireEvent<const char*>(
          mBus, MessageBus::LoadPresetFromString, mPreset->data.c_str()
        );
      }
      //soundwoofer::async::loadPreset(mPreset, [&](soundwoofer::Status status) {
      //  if (status == soundwoofer::SUCCESS) {
      //    MessageBus::fireEvent<const char*>(
      //      mBus, MessageBus::LoadPresetFromString, mPreset->data.c_str()
      //    );
      //  }
      //});
    }
  };
}