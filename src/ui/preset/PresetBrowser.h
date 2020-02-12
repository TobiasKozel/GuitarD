#pragma once
#ifndef GUITARD_HEADLESS
#include "../../ui/ScrollViewControl.h"
#include "../../misc/MessageBus.h"
#include "../../ui/ScrollViewChild.h"
#include "../../../thirdparty/soundwoofer/soundwoofer.h"
#include "./PresetEntryControl.h"

namespace guitard {
  class PresetBrowser : public ScrollViewControl, public ScrollViewChild {
    MessageBus::Bus* mBus = nullptr;
    soundwoofer::SWPresets mPresets;
  public:

    PresetBrowser(MessageBus::Bus* pBus, IGraphics* g) :
      ScrollViewControl(IRECT())
    {
      mBus = pBus;
    }

    void OnInit() override {
      ScrollViewControl::OnInit();
      setFullWidthChildren(true);
      // refresh();
    }

    void onScrollInView() override {
      refresh();
    }

    void refresh() {
      soundwoofer::async::listPresets([&](soundwoofer::Status status) {
        soundwoofer::SWPresets result = soundwoofer::instance().getPresets();
        if (status == soundwoofer::SUCCESS || result.size()) { // The server might fail, but there can be user presets
          mPresets = result;
          clearChildren(true);
          for (auto& i : mPresets) {
            PresetEntryControl* p = new PresetEntryControl(mBus, i);
            appendChild(p);
          }
        }
        GetUI()->SetAllControlsDirty();
        OnResize();
      });
    }

    void onScrollOutView() override {

    }
  };
}
#endif