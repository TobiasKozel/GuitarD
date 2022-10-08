#pragma once
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace soundwoofer {
	static const double PI = 3.1415926535897932384626433832795;

	enum Status {
		SUCCESS = 0,
		ASYNC, // Not an error, just signals the function will run in another thread
		NOT_CACHED,
		GENERIC_ERROR,
		SERVER_ERROR,
		PLUGIN_NAME_NOT_SET,
		WAV_ERROR,
		NOT_IMPLEMENTED,
		JSON_ENCODE_ERROR,
		UNKNOWN_IR, // This happens when a SWImpulse is passed as an argument which is not in mIRList
	};

	/**
	 * Mostly relevant for SWImpulse but SWRig, SWComponent and SWPreset will have this property too
	 */
	enum Source {
		SOUNDWOOFER_SRC = 0, // file will contain a uuid. IR file will either be on the server or the cache folder
		USER_SRC,            // Relative path starting from mIrDirectory
		USER_SRC_ABSOLUTE,   // Absolute path is in the file string will most likely cause issues when loading on a different computer
		EMBEDDED_SRC         // Means there's no actual file only a float buffer
	};

	const std::string TypeMicrophone = "Microphone";
	const std::string TypeCabinet = "Cabinet";

	/**
	 * Struct based on an Impulse Response from the soundwoofer API
	 */
	struct SWImpulse;
	typedef std::shared_ptr<SWImpulse> SWImpulseShared;
	typedef std::vector<SWImpulseShared> SWImpulses;
	struct SWRig;
	typedef std::shared_ptr<SWRig> SWRigShared;
	typedef std::vector<SWRigShared> SWRigs;
	struct SWComponent;
	typedef std::shared_ptr<SWComponent> SWComponentShared;
	typedef std::vector<SWComponentShared> SWComponents;
	struct SWPreset;
	typedef std::shared_ptr<SWPreset> SWPresetShared;
	typedef std::vector<SWPresetShared> SWPresets;

	struct SWImpulse {
		std::string id;
		std::string name;
		std::string micId; // The ID of the mic used to record this
		std::string rig; // The ID of the SWRig used to record this, not a reference to precent circular deps
		std::string file; // This is either a uuid or a path
		Source source = SOUNDWOOFER_SRC; // Internally used
		int micX = 0;
		int micY = 0;
		int micZ = 0;
		std::string description;
		int micPosition;
		std::string userName;
		std::string element;
		size_t channels = 0; // Holds the channel count after loadIR
		size_t length = 0;  // Holds the length in samples for a single channel after loadIR
		float** samples = nullptr;  // Holds the sample data after loadIR
		size_t sampleRate = 0;  // Holds the sample rate after loadIR
		/**
		 * Used internally, if this is true, this means soundwoofer is no longer managing the IR
		 * After calling listIRs all the IRs objects will not be part of soundwoofer
		 */
		bool managed = true;
		bool normalized = false;

		void clearSamples() {
			if (samples == nullptr || source == EMBEDDED_SRC) { return; }
			for (int i = 0; i < channels; i++) {
				delete[] samples[i];
			}
			delete[] samples;
			samples = nullptr;
			channels = length = 0;
			normalized = false;
		}
		~SWImpulse() {
			clearSamples();
		}
	};

	/**
	 * A component can be a microphone or a specific instance of a Cabinet
	 */
	struct SWComponent {
		std::string id;
		std::string name;
		std::string type; // "Microphone", "Cabinet"
		Source source = SOUNDWOOFER_SRC;
		int componentBase = 0; // TODO find out what this represents
		std::string desciption; // TODO there's a typo in the backend
		size_t year = 0;
		std::string url;
		std::string userName;
		std::string brand;
		std::string model;
		std::string baseComponentDescription;
		bool managed = true;
	};

	/**
	 * The rig basically represents a Cabinet
	 * It knows which mics were used and what IRs are available
	 * It also knows which specific instance of a Cabinets are available,
	 * since the same cab can be modified or recorded by different people
	 */
	struct SWRig {
		std::string id;
		std::string name;
		Source source = SOUNDWOOFER_SRC;
		std::string userName;
		std::string description;
		SWComponents components;
		SWComponents microphones;
		SWImpulses impulses;
		bool managed = true;
	};

	/**
	 * Preset Struct Based on the soundwoofer API
	 */
	struct SWPreset {
		std::string name;
		std::string id;
		std::string plugin;
		Source source = SOUNDWOOFER_SRC;
		std::string data;
		std::string version = "-1";
		int rating = 0;
		int ratings = 0;
		std::vector<std::string> tags;
		bool managed = true;
	};
}
