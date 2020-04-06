#pragma once

#include "./soundwooferTypes.h"
  namespace soundwoofer {
  /**
   * This represents the current state of soundwoofer and config
   * Feel free to read data from here but don't alter it
   */
  namespace state {
    /**
     * Generic Objects to categorize IRs with no parent
     */
    SWComponentShared GenericComponent = std::make_shared<SWComponent>(SWComponent{
      "Generic Component", "Generic Component", TypeCabinet, USER_SRC
    });

    SWComponentShared GenericMicrophone = std::make_shared<SWComponent>(SWComponent{
       "Generic Microphone", "Generic Microphone", TypeMicrophone, USER_SRC
    });

    SWRigShared GenericRig = std::make_shared<SWRig>(SWRig{
       "Uncategorized", "Uncategorized", USER_SRC, "Various"
    });


    bool cacheIRs = false;
    bool cachePresets = false;

    std::string pluginName; // Plugin name used to label and filter presets by
    std::string pluginVersion;
    std::string homeDirectory; // Directory for this plugin
    std::string irDirectory; // Directory for user IRs
    std::string irCacheDirectory; // Directory for caching online IRs
    std::string presetCacheDirectory; // Directory for caching online presets
    std::string presetDirectory; // Directory for user IRs

    bool componentListCached = false;
    bool irListCached = false;
    bool rigListCached = false;

    SWImpulses irList;
    SWRigs rigList;
    SWComponents componentList;
    SWPresets presetList;
    std::vector<SWPreset> factoryPresetList;
  };
}
