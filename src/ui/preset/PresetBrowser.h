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
    SoundWoofer& api = SoundWoofer::instance();
    SoundWoofer::SWPresets mPresets;
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
      api.fetchPresets([&](SoundWoofer::Status status) {
        if (status == SoundWoofer::SUCCESS) {
          mPresets = api.getPresets();
          clearChildren(true);
          for (auto& i : mPresets) {
            PresetEntryControl* p = new PresetEntryControl(mBus, &i);
            appendChild(p);
          }
        }
        OnResize();
        GetUI()->SetAllControlsDirty(); // Needed since nothing else triggers a new frame to be rendered
      });
    }

    void onScrollOutView() override {

    }
  };
}
#endif