#pragma once

#include "./soundwooferTypes.h"

#ifndef SOUNDWOOFER_CUSTOM_JSON
  #include "./dependencies/json.hpp"
#endif

#ifdef SOUNDWOOFER_BINARY_PRESETS
  #include "./dependencies/base64.h"
#endif

namespace soundwoofer {
  namespace parse {
#ifndef SOUNDWOOFER_CUSTOM_JSON
    SWImpulseShared ir(nlohmann::json& i) {
      try {
        return SWImpulseShared(new SWImpulse {
          i["id"],
          i["name"],
          i["micId"],
          i["rig"],
          i["file"],
          SOUNDWOOFER_SRC,
          i["micX"], i["micY"], i["micZ"],
          i["description"],
          i["micPosition"],
          i["userName"],
          i["element"]
        });
      }
      catch (...) {
        // TODO some of these values are null which will fail
        // assert(false, "Error parsing IR");
      }
      return SWImpulseShared();
    }

    SWImpulseShared ir(const std::string& data) {
      SWImpulseShared ret;
      if (data.size() <= 2) { return ret; } // means empty, or empty array/object
      nlohmann::json json;
      try {
        json = nlohmann::json::parse(data);
      }
      catch (...) {
        return ret;
      }
      return ir(json);
    }

    SWImpulses irs(std::string& data) {
      auto ret = SWImpulses();
      if (data.size() <= 2) { return ret; } // means empty, or empty array/object
      nlohmann::json json;
      try {
        json = nlohmann::json::parse(data);
      }
      catch (...) {
        return ret;
      }
      for (auto& i : json) {
        ret.push_back(ir(i));
      }
      return ret;
    }

    SWRigShared rig(nlohmann::json& i) {
      try {
        return SWRigShared(new SWRig{
          i["id"],
          i["name"],
          SOUNDWOOFER_SRC,
          i["username"],
          i["description"]
        });
      }
      catch (...) {
        assert(false);
      }
      return SWRigShared();
    }

    SWRigShared rig(const std::string& data) {
      SWRigShared ret;
      if (data.size() <= 2) { return ret; } // means empty, or empty array/object
      nlohmann::json json;
      try {
        json = nlohmann::json::parse(data);
      }
      catch (...) {
        return ret;
      }
      return rig(json);
    }

    SWRigs rigs(const std::string& data) {
      auto ret = SWRigs();
      if (data.size() <= 2) { return ret; }
      nlohmann::json json;
      try {
        json = nlohmann::json::parse(data);
      }
      catch (...) {
        return ret;
      }
      for (auto i : json) {
        ret.push_back(rig(i));
      }
      return ret;
    }

    SWPresetsShared preset(nlohmann::json& i, const std::string pluginName) {
      try {
        SWPreset preset = {
          i["name"],
          i["id"],
          i["plugin"],
          SOUNDWOOFER_SRC,
          i["data"],
          i["version"]
        };
        if (preset.plugin == pluginName) {
          return std::make_shared<SWPreset>(preset);
        }
      }
      catch (...) {}
      return SWPresetsShared();
    }

    SWPresetsShared preset(const std::string data, const std::string pluginName) {
      SWPresetsShared ret;
      if (data.size() <= 2) { return ret; } // means empty, or empty array/object
      nlohmann::json json;
      try {
        json = nlohmann::json::parse(data);
      }
      catch (...) {
        return ret;
      }
      return preset(json, pluginName);
    }

    SWPresets presets(const std::string& data, const std::string pluginName) {
      auto ret = SWPresets();
      if (data.size() <= 2) { return ret; }
      nlohmann::json json;
      try {
        json = nlohmann::json::parse(data);
      }
      catch (...) {
        return ret;
      }
      for (auto i : json) {
        auto p = preset(i, pluginName);
        if (p != nullptr) {
          ret.push_back(p);
        }
      }
      return ret;
    }
#endif
  }

  namespace encode {
#ifndef SOUNDWOOFER_CUSTOM_JSON
    std::string preset(const SWPreset& preset) {
      nlohmann::json json;
      json["name"] = preset.name;
      json["data"] = preset.data;
      json["id"] = preset.id;
      json["plugin"] = preset.plugin;
      return json.dump(4);
    }
#endif
  }
}