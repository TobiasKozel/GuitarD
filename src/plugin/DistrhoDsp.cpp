#include "DistrhoPlugin.hpp"
#include "./DistrhoPluginInfo.h"

#define GUITARD_MAX_DAW_PARAMS DISTRHO_PLUGIN_PARAMETER_COUNT
#define GUITARD_HEADLESS
#define SAMPLE_TYPE_FLOAT
#include "../main/Graph.h"

using namespace guitard;

START_NAMESPACE_DISTRHO

class DistrhoDsp : public Plugin {
public:
	DistrhoDsp() : Plugin(DISTRHO_PLUGIN_PARAMETER_COUNT, 0, DISTRHO_PLUGIN_STATE_COUNT) {
		std::memset(mParameters, 0, sizeof(float) * DISTRHO_PLUGIN_PARAMETER_COUNT);
		mGraph.setParameterManager(&mParamManager);
		std::string homeDir;
		#ifdef unix
			homeDir = getenv("HOME"); // maybe call free on it
			#elif defined(_WIN32)
			#ifdef _MSC_VER // Also make sure to use the multibyte charset for msvc
				char* pValue;
				size_t len;
				errno_t err = _dupenv_s(&pValue, &len, "HOMEDRIVE");
				homeDir = pValue;
				free(pValue);
				err = _dupenv_s(&pValue, &len, "HOMEPATH");
				homeDir.append(pValue);
				free(pValue);
			#else
				homeDir = getenv("HOMEDRIVE");
				homeDir.append(getenv("HOMEPATH"));
			#endif
		#endif

		printf("\n%s\n", homeDir.c_str());
		soundwoofer::setup::setPluginName(DISTRHO_PLUGIN_NAME);
		soundwoofer::setup::setHomeDirectory(homeDir.c_str());
	}

protected:

	#pragma region Plugin initialization

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

	#pragma endregion

	#pragma region DAW State interaction

	String getState(const char* key) const override {
		if (std::strcmp(key, DISTRHO_PLUGIN_STATE_PATCH) == 0) {
			return String();
		}

		if (std::strcmp(key, DISTRHO_PLUGIN_STATE_SCREENSHOT) == 0) {
			return String();
		}

		if (std::strcmp(key, DISTRHO_PLUGIN_STATE_NAME) == 0) {
			return String();
		}

		if (std::strcmp(key, DISTRHO_PLUGIN_STATE_WINDOW_SIZE) == 0) {
			return String();
		}
		return String();
	}

	void setState(const char* key, const char* value) override {
		if (std::strcmp(key, DISTRHO_PLUGIN_STATE_PATCH) == 0) {
			mGraph.deserialize(value);
		}

		if (std::strcmp(key, DISTRHO_PLUGIN_STATE_SCREENSHOT) == 0) {
		}

		if (std::strcmp(key, DISTRHO_PLUGIN_STATE_NAME) == 0) {
		}

		if (std::strcmp(key, DISTRHO_PLUGIN_STATE_WINDOW_SIZE) == 0) {
		}
	}

	float getParameterValue(uint32_t index) const override {
		const ParameterCoupling* couple = mParamManager.getCoupling(index);
		if (couple != nullptr) {
			return couple->getNormalized();
		}
		return 0;
	}

	void setParameterValue(uint32_t index, float value) override {
		ParameterCoupling* couple = mParamManager.getCoupling(index);
		if (couple != nullptr) {
			couple->setFromNormalized(value);
		}
		mParameters[index] = value;
	}

	#pragma endregion

	#pragma region Signal Processing
	void run(const float** inputs, float** outputs, uint32_t frames) override {
		if (mReady) {
			mGraph.ProcessBlock(const_cast<float**>(inputs), outputs, frames);
		}
	}

	void activate() override {
		mGraph.OnTransport();
	}

	void deactivate() override {
		mGraph.OnTransport();
	}

	void sampleRateChanged(const double newSampleRate) override {
		// TODO channel count
		mGraph.OnReset(newSampleRate, 2, 2);
		mReady = true;
	}

	#pragma endregion

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
	ParameterManager mParamManager;
    Graph mGraph;
	float mParameters[DISTRHO_PLUGIN_PARAMETER_COUNT];
	bool mReady = false;
	DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistrhoDsp)
};

Plugin* createPlugin() { return new DistrhoDsp(); }

END_NAMESPACE_DISTRHO
