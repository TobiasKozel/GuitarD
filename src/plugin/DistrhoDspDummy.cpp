#include "DistrhoPlugin.hpp"
#include "./DistrhoPluginInfo.h"

START_NAMESPACE_DISTRHO

// Dummy class to speed up compilation for UI debugging
class DistrhoDsp : public Plugin {
public:
	DistrhoDsp() : Plugin(DISTRHO_PLUGIN_PARAMETER_COUNT, 0, DISTRHO_PLUGIN_STATE_COUNT) { }

protected:

	void initState(uint32_t index, State& state) override {
		switch (index) {
			case 0:
				state.hints = kStateIsBase64Blob | kStateIsOnlyForDSP;
				state.key = DISTRHO_PLUGIN_STATE_PATCH;
				state.label = "Patch";
				break;
			case 1:
				// Not used at the moment
				state.hints = kStateIsHostReadable | kStateIsBase64Blob;
				state.key = DISTRHO_PLUGIN_STATE_SCREENSHOT;
				state.label = "Screenshot";
				break;
			case 2:
				state.hints = kStateIsHostWritable;
				state.key = DISTRHO_PLUGIN_STATE_NAME;
				state.label = "Name";
				break;
			case 3:
				state.hints = kStateIsOnlyForUI;
				state.key = DISTRHO_PLUGIN_STATE_WINDOW_SIZE;
				state.label = "Window size";
				break;
		}
	}

	void initParameter(const uint32_t index, Parameter& parameter) override {
		parameter.name = "Parameter ";
		parameter.name += String(index + 1);
		parameter.symbol = "param_";
		parameter.symbol += String(index + 1);
		// parameter.unit = "v"; // no unit because the parameters are generic
		parameter.hints = kParameterIsAutomatable;
		parameter.ranges.def = 0.0f;
		parameter.ranges.min = 0.0f;
		parameter.ranges.max = 1.0f;
	}



	void run(const float**, float** outputs, uint32_t frames) override {
		for (uint32_t i = 0; i < frames; i++) {
			outputs[0][i] = 0;
			outputs[1][i] = 0;
		}
	}


	// -------------------------------------------------------------------------------------------------------

	#pragma region Plugin Metadata

	int64_t getUniqueId() const override { return d_cconst('g', 't', 'r', 'd'); }
	const char* getLicense() const override { return DISTRHO_PLUGIN_LICENSE; }
	const char* getHomePage() const override { return DISTRHO_PLUGIN_URI; }
	const char* getLabel() const override { return DISTRHO_PLUGIN_NAME; }
	const char* getMaker() const override { return DISTRHO_PLUGIN_BRAND; }
	const char* getDescription() const override {
		return "Node-Based effects processor for electric guitars.";
	}
	uint32_t getVersion() const override {
		return d_version(
			DISTRHO_PLUGIN_VERSION_MAJOR,
			DISTRHO_PLUGIN_VERSION_MINOR,
			DISTRHO_PLUGIN_VERSION_PATCH
		);
	}

	#pragma endregion

private:
	DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistrhoDsp)
};

Plugin* createPlugin() { return new DistrhoDsp(); }

END_NAMESPACE_DISTRHO
