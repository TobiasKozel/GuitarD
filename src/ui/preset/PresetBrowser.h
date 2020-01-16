#pragma once
#include "src/ui/ScrollViewControl.h"
#include "src/misc/MessageBus.h"
#include "src/ui/ScrollViewChild.h"
#include "thirdparty/soundwoofer.h"
#include "PresetEntryControl.h"

using namespace iplug;
using namespace igraphics;


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
    refresh();
  }

  void onScrollInView() override {
    refresh();
  }

  void refresh() {
    if (api.fetchPresets() == SoundWoofer::SUCCESS) {
      mPresets = api.getPresets();
      clearChildren(true);
      for (auto& i : mPresets) {
        PresetEntryControl* p = new PresetEntryControl(mBus, &i);
        appendChild(p);
      }
    }
    OnResize();
  }

  void onScrollOutView() override {
    
  }
};