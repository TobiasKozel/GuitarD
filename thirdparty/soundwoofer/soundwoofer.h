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
     * Sends the preset to the server
     * Doesn't update the cached preset list
     */
    Status sendPreset(const SWPreset preset) {
      if (mPluginName.empty()) { return PLUGIN_NAME_NOT_SET; }

      const std::string serialized = encodePreset(preset);
      if (serialized.empty()) { return JSON_ENCODE_ERROR; }
      return httpPost("/Preset", serialized.c_str(), serialized.size());
    }

#endif

    /**
     * Download and decode a specific IR
     * If it's local IR or cached, it will skip the download
     * The decoded IR will be in SWImpulse::samples
     * Will block, use a lambda as a callback for async
     */
    Status loadIR(SWImpulseShared ir, size_t sampleRate = 0, bool normalize = true) {
      if (ir->samples != nullptr) {
        if (normalize) {
          wave::normalizeIR(ir);
        }
        return SUCCESS; // Already in ram
      }

      Status ret = GENERIC_ERROR;
      // Try loading the wave from disk first
      std::string path;
      if (ir->source == USER_SRC) { path = mIrDirectory + ir->file; }
      if (ir->source == USER_SRC_ABSOLUTE) { path = ir->file; }
      if (ir->source == SOUNDWOOFER_SRC) { path = mIrCacheDirectory + ir->id; }
      ret = wave::loadWaveFile(ir, path, sampleRate, normalize);
      if (ret == SUCCESS) { return SUCCESS; } // Loaded from disk
      if (ir->source != SOUNDWOOFER_SRC) { return UNKNOWN_IR; } // No file and also not a online IR
#ifndef SOUNDWOOFER_NO_API
      // Go look online
      const std::string result = httpGet("/File/Download/" + ir->file);
      if (result.empty()) { return SERVER_ERROR; }
      ret = wave::loadWaveMemory(ir, result.c_str(), result.size(), sampleRate, normalize);
      if (ret == SUCCESS && mCacheIRs) {
        // Cache the file
        file::writeFile((mIrCacheDirectory + ir->id).c_str(), result.c_str(), result.size());
      }
#endif
      return ret;
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
     * Will add a generic Mic, amp, factory presets and factory IRs
     */
    void resetIRs() {
      flushIRBuffers();
      for (auto& i : mComponentList) { i->managed = false; }
      mComponentList.clear();
      for (auto& i : mRigList) { i->managed = false; }
      mRigList.clear();
      for (auto& i : mIRlist) { i->managed = false; }
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