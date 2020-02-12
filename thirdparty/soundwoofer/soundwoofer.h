#pragma once

#ifndef SOUNDWOOFER_CUSTOM_JSON
  #include "./dependencies/json.hpp"
#endif

// #define SOUNDWOOFER_NO_API

#ifndef SOUNDWOOFER_NO_API
  #ifndef SOUNDWOOFER_CUSTOM_HTTP
    // #define CPPHTTPLIB_OPENSSL_SUPPORT
    #include "./dependencies/httplib.h"
  #endif
#endif

#ifdef SOUNDWOOFER_BINARY_PRESETS
  #include "./dependencies/base64.h"
#endif

#ifndef SOUNDWOOFER_CUSTOM_WAVE
  #define DR_WAV_IMPLEMENTATION
  #include "./dependencies/dr_wav.h"
#endif

#include "./soundwooferTypes.h"
#include "./soundwooferFile.h"
#include "./soundwooferWave.h"


namespace soundwoofer {
  class SW {
    /**
     * Generic Objects to categorize IRs with no parent
     */
    SWComponentShared GenericComponent;
    SWComponentShared GenericMicrophone;
    SWRigShared GenericRig;

#ifndef SOUNDWOOFER_NO_API
    // const std::string BACKEND_URL = "svenssj.tech";
    const std::string API_VERSION = "0.1";
    const std::string BACKEND_URL = "localhost";
    const int BACKEND_PORT = 5000;
#endif
    bool mCacheIRs = false;
    bool mCachePresets = false;

    int mTargetSampleRate = 0;

    std::string mPluginName; // Plugin name used to label and filter presets by
    std::string mPluginVersion;
    std::string mHomeDirectory; // Directory for this plugin
    std::string mIrDirectory; // Directory for user IRs
    std::string mIrCacheDirectory; // Directory for caching online IRs
    std::string mPresetCacheDirectory; // Directory for caching online presets
    std::string mPresetDirectory; // Directory for user IRs

    SWImpulses mIRlist;
    SWRigs mRigList;
    SWComponents mComponentList;
    SWPresets mPresetList;
    std::vector<SWPreset> mFactoryPresetList;

    SW() {
      GenericComponent = std::make_shared<SWComponent>(SWComponent{
        "Generic Component", "Generic Component", TypeCabinet, USER_SRC
        });
      GenericMicrophone = std::make_shared<SWComponent>(SWComponent{
        "Generic Microphone", "Generic Microphone", TypeMicrophone, USER_SRC
        });
      GenericRig = std::make_shared<SWRig>(SWRig{
        "Uncategorized", "Uncategorized", USER_SRC, "Various"
        });
      resetIRs();
      // TODO seems like the object will be destroyed once even though it's a singleton
    }
  public:

    /**
     * Set this first, it's needed for caching and for online presets
     */
    void setPluginName(const std::string& name) {
      mPluginName = name;
      mCacheIRs = true;
      mCachePresets = true;
    }

    void setPluginVersion(const std::string& version) {
      mPluginVersion = version;
    }

    /**
     * Any non 0 value will do resampling when calling load IR
     */
    void setTargetSampleRate(int sampleRate) {
      mTargetSampleRate = sampleRate;
    }

    /**
     * Sets the location for data like caches etc
     */
    Status setHomeDirectory(std::string path) {
      if (mPluginName.empty()) { return PLUGIN_NAME_NOT_SET; }
      path += file::PATH_DELIMITER + mPluginName + file::PATH_DELIMITER;
      mHomeDirectory = path;
      mPresetDirectory = path + "presets" + file::PATH_DELIMITER;
      mIrDirectory = path + "irs" + file::PATH_DELIMITER;
      mPresetCacheDirectory = path + "preset_cache" + file::PATH_DELIMITER;
      mIrCacheDirectory = path + "ir_cache" + file::PATH_DELIMITER;
      bool ok = true;
      ok = file::createFolder(path.c_str()) != SUCCESS ? false : ok;
#ifndef SOUNDWOOFER_NO_API
      ok = file::createFolder(mPresetCacheDirectory.c_str()) != SUCCESS ? false : ok;
      ok = file::createFolder(mIrCacheDirectory.c_str()) != SUCCESS ? false : ok;
#endif
      ok = file::createFolder(mPresetDirectory.c_str()) != SUCCESS ? false : ok;
      ok = file::createFolder(mIrDirectory.c_str()) != SUCCESS ? false : ok;
      return ok ? SUCCESS : GENERIC_ERROR;
    }

    std::string getUserIrPath() const {
      return mIrDirectory;
    }

    std::string getUserPresetPath() const {
      return mPresetDirectory;
    }

#ifndef SOUNDWOOFER_NO_API
    std::string getCacheIrPath() const {
      return mIrCacheDirectory;
    }

    std::string getCachePresetPath() const {
      return mPresetCacheDirectory;
    }
#endif

    /**
     * Gets a list of all the Impulse responses from the server and the local folder
     * Will also call clearCachedIRs()
     */
    Status listIRs() {
      resetIRs();

      // This will cunstruct a generic IR with no cab or mic as a parent
      auto addToUncategorized = [&](file::FileInfo& info) {
        if (!file::isWaveName(info.name)) { return; }
        SWImpulseShared ir(new SWImpulse{ "CHECKSUM", info.name, GenericMicrophone->id, GenericRig->id, info.relative, USER_SRC });
        GenericRig->impulses.push_back(ir);
      };

      if (!mIrDirectory.empty()) { // Gather the user IRs in the IR directory
        auto cabLevel = file::scanDir(mIrDirectory);
        for (auto i : cabLevel) {
          if (i.isFolder) {
            SWRigShared rig(new SWRig{ i.name, i.name, USER_SRC });
            auto micLevel = file::scanDir(i);
            for (auto j : micLevel) {
              bool micExists = false;
              for (auto& m : mComponentList) {
                if (m->name == j.name) {
                  micExists = true;
                  break;
                }
              }
              SWComponentShared mic(new SWComponent{ j.name, j.name, TypeMicrophone, USER_SRC });
              rig->microphones.push_back(mic);
              if (!micExists) { mComponentList.push_back(mic); }  // GLOBAL
              if (j.isFolder) {
                auto posLevel = file::scanDir(j);
                for (auto k : posLevel) {
                  if (!k.isFolder && file::isWaveName(k.name)) {
                    SWImpulseShared ir(new SWImpulse{ "CHECKSUM", k.name, mic->id, rig->id, k.relative, USER_SRC });
                    mIRlist.push_back(ir); // GLOBAL
                    rig->impulses.push_back(ir);
                  }
                }
              }
              else { addToUncategorized(j); } // At mic level
            }
            mRigList.push_back(rig); // GLOBAL
          }
          else { addToUncategorized(i); } // At Cabinet level
        }
      }

#ifndef SOUNDWOOFER_NO_API
      std::string data = httpGet("/Impulse");
      if (data.empty()) { return SERVER_ERROR; }
      mIRlist = parseIRs(data);
#endif
      //data = httpGet("/Rig");
      //if (data.empty()) { return SERVER_ERROR; }
      //mCabList = parseRigs(data);
      return SUCCESS;
      // TODO assign them to the cabs an mics
    }



    /**
     * Fetches presets from the server and the local folder
     */
    Status listPresets() {
      mPresetList.clear();

      for (auto i : mFactoryPresetList) { // Gather factory presets
        mPresetList.push_back(std::make_shared<SWPreset>(i));
      }

      if (!mPresetDirectory.empty()) { // Gather user presets
        auto files = file::scanDir(mPresetDirectory);
        for (auto i : files) {
          if (!i.isFolder && file::isJSONName(i.name)) {
            SWPresetsShared preset(new SWPreset{ i.name, i.name, mPluginName, USER_SRC });
            mPresetList.push_back(preset);
          }
        }
      }
#ifndef SOUNDWOOFER_NO_API
      // Gather soundwoofer presets
      if (mPluginName.empty()) { return PLUGIN_NAME_NOT_SET; }
      std::string data = httpGet("/Preset");
      if (data.empty()) { return SERVER_ERROR; }
      SWPresets online = parsePresets(data);
      mPresetList.insert(mPresetList.end(), online.begin(), online.end());
#endif
      return SUCCESS;
    }



#ifndef SOUNDWOOFER_NO_API
    /**
     * Assembles a SWPreset and sends it to the server
     * Doesn't update the cached presets
     */
    Status sendPreset(const std::string name, const char* data, const size_t length) {
      if (mPluginName.empty()) { return PLUGIN_NAME_NOT_SET; }
      SWPreset preset = {
        name,
        file::generateUUID(),
        mPluginName
      };
      preset.data.append(data, length);
      return sendPreset(preset);
    }


    /**
     * Sends the preset to the server
     * Doesn't update the cached preset list
     */
    Status sendPreset(const SWPreset preset) {
      if (mPluginName.empty()) { return PLUGIN_NAME_NOT_SET; }

      const std::string serialized = encodePreset(preset);
      if (serialized.empty()) { return JSON_ENCODE_ERROR; }
      return httpPost("/Preset", serialized.c_str(), serialized.size());
    }


    /**
     * Load an IR from an id (Only works for Cloud IRs)
     */
    Status loadIR(std::string fileId) {
      for (auto& i : mIRlist) {
        if (fileId == i->file) {
          return loadIR(i);
        }
      }
      return GENERIC_ERROR;
    }
#endif

    /**
     * Download and decode a specific IR
     * If it's local IR or cached, it will skip the download
     * The decoded IR will be in SWImpulse::samples
     * Will block, use a lambda as a callback for async
     */
    Status loadIR(SWImpulseShared outsideIR) {
      SWImpulseShared ir = nullptr;
      bool unknownIR = true;
      for (auto& i : mIRlist) {
        if (i == outsideIR) { // Go look if we know the IR
          ir = i;
          unknownIR = false;
          break;
        }
      }
      if (unknownIR) { ir = outsideIR; } // We'll still try to load an ir
      if (outsideIR->samples != nullptr) { return SUCCESS; } // already in ram

      Status load = GENERIC_ERROR;
      load = loadWave(ir);
      if (load == SUCCESS) { return SUCCESS; }
      if (unknownIR || ir->source != SOUNDWOOFER_SRC) { return UNKNOWN_IR; }
#ifndef SOUNDWOOFER_NO_API
      const std::string result = httpGet("/File/Download/" + ir->file);
      if (result.empty()) { return SERVER_ERROR; }
      load = loadWave(ir, result.c_str(), result.size());
      if (load == SUCCESS && mCacheIRs) { // Cache the file
        file::writeFile((mIrCacheDirectory + ir->id).c_str(), result.c_str(), result.size());
      }
#endif
      return load;
    }

    Status loadPreset(SWPresetsShared preset) {
      if (!preset->data.empty()) { return SUCCESS; }
      if (preset->source == SOUNDWOOFER_SRC) {
        const std::string result = httpGet("/Preset/" + preset->id);
        if (result.empty()) { return SERVER_ERROR; }
        const auto dl = parsePresets(result);
        if (dl.size() != 1) { return SERVER_ERROR; }
        preset->data = dl.at(0)->data;
        return SUCCESS;
      }
      std::ifstream file(mPresetDirectory + file::PATH_DELIMITER + preset->name);
      const std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      preset->data = str;
      return SUCCESS;
    }


#ifndef SOUNDWOOFER_NO_API
    SWImpulseShared getIR(std::string fileId) {
      for (auto& i : mIRlist) {
        if (fileId == i->file) {
          return i;
        }
      }
      return nullptr;
    }
#endif

    SWImpulses& getIRs() {
      return mIRlist;
    }

    SWRigs& getRigs() {
      return mRigList;
    }

    SWPresets& getPresets() {
      return mPresetList;
    }

    /**
     * Will clear out all IR buffers currently in ram
     */
    void flushIRBuffers() {
      for (auto& i : mIRlist) {
        i->clearSamples();
      }
    }

#ifndef SOUNDWOOFER_NO_API
    /**
     * Will clear out the disk cached IRs
     */
    void clearIRCache() {
      // TODO
    }
#endif



    /**
     * Singleton stuff
     */
    SW(const SW&) = delete;
    SW& operator = (const SW&) = delete;

    static SW& instance() {
      static SW instance;
      return instance;
    }

    virtual ~SW() {
      flushIRBuffers();
      //if (mThread.joinable()) {
      //  mThread.join();
      //}
    }

  private:
    virtual SWImpulses parseIRs(std::string& data) {
      auto ret = SWImpulses();
      if (data.size() <= 2) { return ret; }
#ifndef SOUNDWOOFER_CUSTOM_JSON
      auto json = nlohmann::json::parse(data);
      for (auto i : json) {
        try {
          ret.push_back(
            SWImpulseShared(new SWImpulse{
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
              }));
        }
        catch (...) {
          // TODO some of these values are null which will fail
          int test = 0;
          // assert(false, "Error parsing IR");
        }
      }
#else
      assert(false); // You need to override this function if you want to use a different json parser!
#endif
      return ret;
    }

    virtual SWRigs parseRigs(const std::string& data) {
      auto ret = SWRigs();
      if (data.size() <= 2) { return ret; }
#ifndef SOUNDWOOFER_CUSTOM_JSON
      auto json = nlohmann::json::parse(data);
      for (auto i : json) {
        try {
          ret.push_back(SWRigShared(new SWRig{
            i["id"],
            i["name"],
            SOUNDWOOFER_SRC,
            i["username"],
            i["description"]
            }));
        }
        catch (...) {
          assert(false);
        }
      }
#else
      assert(false); // You need to override this function if you want to use a different json parser!
#endif
      return ret;
    }

    virtual SWPresets parsePresets(const std::string& data) {
      auto ret = SWPresets();
      if (data.size() <= 2) { return ret; }
#ifndef SOUNDWOOFER_CUSTOM_JSON
      auto json = nlohmann::json::parse(data);
      for (auto i : json) {
        try {
          SWPreset preset = {
            i["name"],
            i["id"],
            i["plugin"],
            SOUNDWOOFER_SRC,
            i["data"],
            i["version"]
          };
          if (preset.plugin == mPluginName) {
            ret.push_back(std::make_shared<SWPreset>(preset));
          }
        }
        catch (...) {
          assert(false);
        }
      }
#else
      assert(false); // You need to override this function if you want to use a different json parser!
#endif
      return ret;
    }

    virtual std::string encodePreset(const SWPreset& preset) {
      std::string ret;
#ifndef SOUNDWOOFER_CUSTOM_JSON
      nlohmann::json json;
      json["name"] = preset.name;
      json["data"] = preset.data;
      json["id"] = preset.id;
      json["plugin"] = preset.plugin;
      ret = json.dump();
#else
      assert(false); // You need to override this function if you want to use a different json parser!
#endif
      return ret;
    }

    /**
     * Will do a synchronous http request to the url + port + the endpoint parameter
     * Returns a string of the response body or an empty string for any error
     */
#ifndef SOUNDWOOFER_NO_API
    virtual std::string httpGet(const std::string endpoint) {
#ifndef SOUNDWOOFER_CUSTOM_HTTP
      httplib::Client cli(BACKEND_URL, BACKEND_PORT);
      auto res = cli.Get(endpoint.c_str());
      if (res && res->status == 200) {
        return res->body;
      }
#else
      assert(false); // You need to override this function if you want to use a different http lib!
#endif
      return std::string();
    }

    virtual Status httpPost(const std::string endpoint, const char* data, const size_t length, const std::string mime = "application/json") {
#ifndef SOUNDWOOFER_CUSTOM_HTTP
      httplib::Client cli(BACKEND_URL, BACKEND_PORT);
      std::string body;
      body.append(data, length);
      // body += "\0"; // make sure it's null terminated
      auto res = cli.Post(endpoint.c_str(), body, mime.c_str());
      if (res && res->status == 200) {
        return SUCCESS;
      }
      return SERVER_ERROR;
#else
      return NOT_IMPLEMENTED;
#endif
    }
#endif

    /**
     * Will load the wav file to a float buffer in the SWImpulse struct
     */
    virtual Status loadWave(SWImpulseShared& ir, const char* waveData = nullptr, const size_t length = 0) {
#ifndef SOUNDWOOFER_CUSTOM_WAVE
      drwav wav;
      if (waveData != nullptr) { // Means the wave is in memory and not on disk
        if (!drwav_init_memory(&wav, waveData, length, nullptr)) {
          return WAV_ERROR;
        }
      }
      else {
        std::string path;
        if (ir->source == USER_SRC) { path = mIrDirectory + ir->file; }
        if (ir->source == USER_SRC_ABSOLUTE) { path = ir->file; }
        if (ir->source == SOUNDWOOFER_SRC) { path = mIrCacheDirectory + ir->id; }

        if (!drwav_init_file(&wav, path.c_str(), nullptr)) {
          return NOT_CACHED; // This means we'll need to go online and get the IR
        }
        if (ir->source == USER_SRC) { // Hash the file so we can go look for it if the file is missing on load
          ir->id = file::hashFile(mIrDirectory + ir->file);
        }
      }

      // interleaved buffer
      float* pSampleData = static_cast<float*>(malloc(
        static_cast<size_t>(wav.totalPCMFrameCount)* wav.channels * sizeof(float)
      ));

      ir->length = drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, pSampleData);
      if (ir->length == 0) { return WAV_ERROR; }
      ir->sampleRate = wav.sampleRate;
      ir->channels = wav.channels;
      ir->samples = new float* [ir->channels];
      for (int c = 0; c < ir->channels; c++) {
        ir->samples[c] = new float[ir->length];
      }
      for (int s = 0; s < ir->length * ir->channels; s++) {
        // slow deinterleaving, but it works
        const int channel = s % ir->channels;
        const size_t sample = s / ir->channels;
        ir->samples[channel][sample] = pSampleData[s];
      }
      free(pSampleData);
      drwav_uninit(&wav);
      if (mTargetSampleRate > 0 && ir->sampleRate != mTargetSampleRate) {
        wave::WindowedSincResampler<float, float> resampler(ir->sampleRate, mTargetSampleRate);
        float** resampled = new float*[ir->channels];
        size_t sampleCount = 0;
        const size_t channelCount = ir->channels;
        for (size_t i = 0; i < channelCount; i++) {
          sampleCount = resampler.resample(ir->samples[i], ir->length, &(resampled[i]));
        }
        ir->clearSamples();
        ir->length = sampleCount;
        ir->channels = channelCount;
        ir->samples = resampled;
      }
      return SUCCESS;
#else
      return NOT_IMPLEMENTED;
#endif
    }

    /**
     * Will add a generic Mic, amp, factory presets and factory IRs
     */
    void resetIRs() {
      flushIRBuffers();
      mComponentList.clear();
      mRigList.clear();
      mIRlist.clear();
      GenericRig->impulses.clear();
      GenericRig->components.clear();
      GenericRig->impulses.clear();
      mComponentList.push_back(GenericComponent);
      mComponentList.push_back(GenericMicrophone);
      mRigList.push_back(GenericRig);
      GenericRig->components.push_back(GenericComponent);
      GenericRig->components.push_back(GenericMicrophone);
    }
  };

  SW& instance() {
    return SW::instance();
  }

  /**
   * Create IRs which live outside the SoundWoofer domain
   */
  SWImpulseShared createGenericIR(
    std::string name, float** samples, size_t length, size_t channels = 1,
    size_t sampleRate = 48000, Source source = EMBEDDED_SRC)
  {
    SWImpulseShared ret(new SWImpulse());
    ret->name = name;
    ret->id = "Factory-" + name;
    ret->samples = samples;
    ret->length = length;
    ret->channels = channels;
    ret->sampleRate = sampleRate;
    ret->source = source;
    return ret;
  }

}

#ifndef SOUNDWOOFER_DO_ASYNC
  #include "./soundwooferAsync.h"
#endif