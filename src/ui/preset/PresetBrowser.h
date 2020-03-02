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
    soundwoofer::async::Callback mCallback =
      std::make_shared<soundwoofer::async::CallbackFunc>(
      [&](soundwoofer::Status s) {
        this->putPresets();
      }
    );
  public:

    PresetBrowser(MessageBus::Bus* pBus) : ScrollViewControl(IRECT()) {
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

    void OnDetached() override {
      IControl::OnDetached();
    }

    void putPresets() {
      soundwoofer::SWPresets result = soundwoofer::preset::get();
      if (result.size()) {
        mPresets = result;
        clearChildren(true);
        for (auto& i : mPresets) {
          PresetEntryControl* p = new PresetEntryControl(mBus, i);
          appendChild(p);
        }
      }
      GetUI()->SetAllControlsDirty();
      OnResize();
    }

    void refresh() {
      //soundwoofer::preset::list();
      //putPresets();
      soundwoofer::async::preset::list(mCallback);
    }

    void onScrollOutView() override {

    }
  };
}
#endif