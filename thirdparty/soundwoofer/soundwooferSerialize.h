#pragma once

#include "./soundwooferState.h"
#include "./soundwooferTypes.h"
#include "./dependencies/json.hpp"

namespace soundwoofer {
	namespace parse {
#ifndef SOUNDWOOFER_CUSTOM_JSON
		SWImpulseShared ir(nlohmann::json& i);

		SWRigShared rig(nlohmann::json& i);

		SWComponentShared component(nlohmann::json& i);

		SWPresetShared preset(nlohmann::json& i);
#endif
		SWImpulseShared ir(const std::string& data);

		SWImpulses irs(std::string& data);

		SWRigShared rig(const std::string& data);

		SWRigs rigs(const std::string& data);

		SWComponentShared component(const std::string& data);

		SWComponents components(const std::string& data);

		SWPresetShared preset(const std::string data);

		SWPresets presets(const std::string& data);
	}

	namespace encode {
#ifndef SOUNDWOOFER_CUSTOM_JSON
		std::string preset(const SWPresetShared preset) {
			nlohmann::json json;
			json["name"] = preset->name;
			json["data"] = preset->data;
			json["id"] = preset->id;
			json["plugin"] = preset->plugin;
			return json.dump(4);
		}
#endif
	}
}

#include "./soundwooferSerializeImpl.h"