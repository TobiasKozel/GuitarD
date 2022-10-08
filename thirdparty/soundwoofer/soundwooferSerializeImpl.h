#pragma once

#include "./soundwooferSerialize.h"

#ifndef SOUNDWOOFER_CUSTOM_JSON
	#include "./dependencies/json.hpp"
#endif

#ifdef SOUNDWOOFER_BINARY_PRESETS
	#include "./dependencies/base64.h"
#endif

namespace soundwoofer {
	namespace parse {
		SWImpulseShared ir(nlohmann::json& i) {
			try {
				SWImpulse obj;
				// We need these or else the ir can't be used
				obj.id = i.at("id").get<std::string>();
				obj.name = i.at("name").get<std::string>();
				obj.micId = i.at("micId").get<std::string>();
				obj.rig = i.at("rig").get<std::string>();
				obj.file = i.at("file").get<std::string>();
				obj.source = SOUNDWOOFER_SRC;
				// these are optional
				if (!i.at("micX").is_null()) obj.micX = i.at("micX").get<int>();
				if (!i.at("micY").is_null()) obj.micY = i.at("micY").get<int>();
				if (!i.at("micZ").is_null()) obj.micZ = i.at("micZ").get<int>();
				if (!i.at("description").is_null()) obj.description = i.at("description").get<std::string>();
				if (!i.at("micPosition").is_null()) obj.micPosition = i.at("micPosition").get<int>();
				if (!i.at("userName").is_null()) obj.userName = i.at("userName").get<std::string>();
				if (!i.at("element").is_null()) obj.element = i.at("element").get<std::string>();
				return std::make_shared<SWImpulse>(obj);
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
				SWRig obj;
				obj.name = i.at("name").get<std::string>();
				obj.id = i.at("id").get<std::string>();
				obj.source = SOUNDWOOFER_SRC;
				if (!i.at("desciption").is_null()) obj.description = i.at("desciption").get<std::string>();
				if (!i.at("userName").is_null()) obj.userName = i.at("userName").get<std::string>();
				return std::make_shared<SWRig>(obj);
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

		SWComponentShared component(nlohmann::json& i) {
			try {
				SWComponent obj;
				obj.id = i.at("id").get<std::string>();
				obj.name = i.at("name").get<std::string>();
				obj.componentBase = i.at("componentBase").get<int>();
				obj.source = SOUNDWOOFER_SRC;
				return std::make_shared<SWComponent>(obj);
			}
			catch (...) {
				assert(false);
			}
			return SWComponentShared();
		}

		SWComponentShared component(const std::string& data) {
			SWComponentShared ret;
			if (data.size() <= 2) { return ret; } // means empty, or empty array/object
			nlohmann::json json;
			try {
				json = nlohmann::json::parse(data);
			}
			catch (...) {
				return ret;
			}
			return component(json);
		}

		SWComponents components(const std::string& data) {
			auto ret = SWComponents();
			if (data.size() <= 2) { return ret; }
			nlohmann::json json;
			try {
				json = nlohmann::json::parse(data);
			}
			catch (...) {
				return ret;
			}
			for (auto i : json) {
				ret.push_back(component(i));
			}
			return ret;
		}

		SWPresetShared preset(nlohmann::json& i) {
			try {
				SWPreset preset = {
					i["name"],
					i["id"],
					i["plugin"],
					SOUNDWOOFER_SRC
				};
				preset.data = i["data"];
				preset.version = i["version"];
				preset.rating = i["rating"];
				preset.ratings = i["ratings"];
				if (preset.plugin == state::pluginName) {
					return std::make_shared<SWPreset>(preset);
				}
			}
			catch (...) {}
			return SWPresetShared();
		}

		SWPresetShared preset(const std::string data) {
			SWPresetShared ret;
			if (data.size() <= 2) { return ret; } // means empty, or empty array/object
			nlohmann::json json;
			try {
				json = nlohmann::json::parse(data);
			}
			catch (...) {
				return ret;
			}
			return preset(json);
		}

		SWPresets presets(const std::string& data) {
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
				auto p = preset(i);
				if (p != nullptr) {
					ret.push_back(p);
				}
			}
			return ret;
		}
	}
}