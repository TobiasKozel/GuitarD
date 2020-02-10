#pragma once

#ifndef SOUNDWOOFER_CUSTOM_JSON
  #include "json.hpp"
#endif

#define SOUNDWOOFER_NO_API

#ifndef SOUNDWOOFER_NO_API
  #ifndef SOUNDWOOFER_CUSTOM_HTTP
    // #define CPPHTTPLIB_OPENSSL_SUPPORT
    #include "httplib.h"
  #endif
#endif

#ifndef SOUNDWOOFER_CUSTOM_WAVE
  #define DR_WAV_IMPLEMENTATION
  #include "dr_wav.h"
#endif

#ifndef SOUNDWOOFER_CUSTOM_DIR
  #ifdef _WIN32
    #include "./dirent/dirent.h"
  #else
    #include "dirent.h"
  #endif
#endif

#ifdef _WIN32
  #include <sys/stat.h>
#endif

#include <iostream>
#include <sstream>
#include <fstream>
#include <mutex>
#include <iomanip>

class SoundWoofer {
public:
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
   * Soundwoofer structs
   */

  /**
   * Struct based on an Impulse Response from the soundwoofer API
   */
  struct SWImpulse {
    std::string id;
    std::string name;
    std::string micId;
    std::string rig;
    std::string file; // This is either a uuid or a path
    int micX = 0;
    int micY = 0;
    int micZ = 0;
    std::string description;
    int micPosition;
    std::string userName;
    std::string element;
    bool local = false;
    int channels = 0;
    int length = 0;
    float** samples = nullptr;
  };

  const std::string TypeMicrophone = "Microphone";
  const std::string TypeCabinet = "Cabinet";

  struct SWComponent {
    std::string id;
    std::string name;
    std::string type; // "Microphone", "Cabinet"
    int componentBase = 0;
    std::string desciption; // TODO there's a typo in the backend
    int year = 0;
    std::string url;
    std::string userName;
    std::string brand;
    std::string model;
    std::string baseComponentDescription;
    bool local = false;

  };

  struct SWRig {
    std::string id;
    std::string name;
    std::string description;
    std::string username;
    std::vector<SWComponent> components;
    std::vector<SWComponent> microphones;
    std::vector<SWImpulse> impulses;
  };

  /**
   * Preset Struct Based on the soundwoofer API
   */
  struct SWPreset {
    std::string name;
    std::string id;
    std::string plugin;
    std::string data;
    std::string version = "-1";
    std::vector<std::string> tags;
    bool local = false; // This means the preset is not from the server
  };

  /**
   * Struct used to index directories
   */
  struct FileInfo {
    std::string name;
    std::string relative;
    std::string absolute;
    bool isFolder;
    std::vector<FileInfo> children;
  };


  typedef std::vector<SWImpulse> SWImpulses;
  typedef std::vector<SWRig> SWRigs;
  typedef std::vector<SWComponent> SWComponents;
  typedef std::vector<SWPreset> SWPresets;

  /**
   * Callback for async operations which provides a status code
   */
  typedef std::function<void(Status)> Callback;


private:
  const SWComponent GenericComponent = { "Generic Component", "Generic Component", TypeCabinet };
  const SWComponent GenericMicrophone = SWComponent{ "Generic Microphone", "Generic Microphone", TypeMicrophone };
  const SWRig UncategorizedRig = SWRig {
    "Uncategorized", "Uncategorized", "", "Various", { GenericMicrophone }, { GenericComponent }
  };

  const std::string PATH_DELIMITER =
#ifdef _WIN32
    "\\";
#else
    "/";
#endif

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

  /**
   * A function to generalize most tasks used in here
   * Will provide a status code
   */
  typedef std::function<Status()> Task;

  /**
   * Bundles together a task and a callback to call after a task has been finished
   */
  struct TaskBundle {
    Callback callback;
    Task task;
  };

  /**
   * Tasks will be queued up here and processed from back to front by mThread
   */
  std::vector<TaskBundle> mQueue;

  /**
   * This will mutex the mQueue
   */
  std::mutex mMutex;
  std::thread mThread;
  bool mThreadRunning = false;

  SoundWoofer() {
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
    path += PATH_DELIMITER + mPluginName + PATH_DELIMITER;
    mHomeDirectory = path;
    mPresetDirectory = path + "presets" + PATH_DELIMITER;
    mIrDirectory = path + "irs" + PATH_DELIMITER;
    mPresetCacheDirectory = path + "preset_cache" + PATH_DELIMITER;
    mIrCacheDirectory = path + "ir_cache" + PATH_DELIMITER;
    bool ok = true;
    ok = createFolder(path.c_str()) != SUCCESS ? false : ok;
#ifndef SOUNDWOOFER_NO_API
    ok = createFolder(mPresetCacheDirectory.c_str()) != SUCCESS ? false : ok;
    ok = createFolder(mIrCacheDirectory.c_str()) != SUCCESS ? false : ok;
#endif
    ok = createFolder(mPresetDirectory.c_str()) != SUCCESS ? false : ok;
    ok = createFolder(mIrDirectory.c_str()) != SUCCESS ? false : ok;
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
    flushIRBuffers();
    mRigList.clear();
    mComponentList.clear();
    mComponentList.push_back(GenericComponent);
    mComponentList.push_back(GenericMicrophone);
    mIRlist.clear();
    SWRig uncategorized = UncategorizedRig;

    auto addToUncategorized = [&](FileInfo& info) {
      if (!isWaveName(info.name)) { return; }
      SWImpulse ir = {"CHECKSUM", info.name, GenericMicrophone.id, uncategorized.id, info.relative };
      ir.local = true;
      uncategorized.impulses.push_back(ir);
    };

    if (!mIrDirectory.empty()) { // Gather the user IRs in the IR directory
      auto cabLevel = scanDir(mIrDirectory);
      for (auto i : cabLevel) {
        if (i.isFolder) {
          SWRig rig = { i.name, i.name };
          auto micLevel = scanDir(i);
          for (auto j : micLevel) {
            bool micExists = false;
            for (auto& m : mComponentList) {
              if (m.name == j.name) {
                micExists = true;
                break;
              }
            }
            SWComponent mic = { j.name, j.name, TypeMicrophone };
            mic.local = true;
            rig.microphones.push_back(mic);
            if (!micExists) { mComponentList.push_back(mic); }  // GLOBAL
            if (j.isFolder) {
              auto posLevel = scanDir(j);
              for (auto k : posLevel) {
                if (!k.isFolder && isWaveName(k.name)) {
                  SWImpulse ir = {"CHECKSUM", k.name, mic.id, rig.id, k.relative};
                  ir.local = true;
                  mIRlist.push_back(ir); // GLOBAL
                  rig.impulses.push_back(ir);
                }
              }
            } else { addToUncategorized(j); } // At mic level
          }
          mRigList.push_back(rig); // GLOBAL
        } else { addToUncategorized(i); } // At Cabinet level
      }
    }

    mRigList.push_back(uncategorized); // GLOBAL

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

  Status listIRs(const Callback callback) {
    startAsync([&]() {
      return this->listIRs();
    }, callback);
    return ASYNC;
  }

  /**
   * Fetches presets from the server and the local folder
   */
  Status listPresets() {
    mPresetList.clear();
    if (!mPresetDirectory.empty()) {
      auto files = scanDir(mPresetDirectory);
      for (auto i : files) {
        if (!i.isFolder && isJSONName(i.name)) {
          SWPreset preset = { i.name, i.name, mPluginName };
          preset.local = true;
          mPresetList.push_back(preset);
        }
      }
    }
#ifndef SOUNDWOOFER_NO_API
    if (mPluginName.empty()) { return PLUGIN_NAME_NOT_SET; }
    std::string data = httpGet("/Preset");
    if (data.empty()) { return SERVER_ERROR; }
    mPresetList = parsePresets(data);
#endif
    return SUCCESS;
  }

  Status fetchPresets(const Callback callback) {
    startAsync([&]() {
      return this->listPresets();
    }, callback);
    return ASYNC;
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
      generateUUID(),
      mPluginName
    };
    preset.data.append(data, length);
    return sendPreset(preset);
  }

  /**
   * Sends the preset to the server
   * Doesn't update the cached preset list
   */
  Status sendPreset(SWPreset &preset) {
    if (mPluginName.empty()) { return PLUGIN_NAME_NOT_SET; }
    const std::string serialized = encodePreset(preset);
    if (serialized.empty()) { return JSON_ENCODE_ERROR; }
    return httpPost("/Preset", serialized.c_str(), serialized.size());
  }

  Status sendPreset(SWPreset& preset, Callback callback) {
    startAsync([&]() {
      return sendPreset(preset);
    }, callback);
    return ASYNC;
  }

  /**
   * Load an IR from an id (Only works for Cloud IRs)
   */
  Status loadIR(std::string id) {
    for (auto &i : mIRlist) {
      if (id == i.id) {
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
  Status loadIR(SWImpulse& ir) {
    SWImpulse* internalIr = nullptr;
    for (auto& i : mIRlist) {
      if (i.file == ir.file) { internalIr = &i; }
    }
    if (internalIr == nullptr) { return UNKNOWN_IR; }
    if (internalIr->samples != nullptr) { return SUCCESS; } // already in ram

    Status load = GENERIC_ERROR;
    load = loadWave(ir);
    if (load == SUCCESS) { return SUCCESS; }

#ifndef SOUNDWOOFER_NO_API
    const std::string result = httpGet("/File/Download/" + ir.file); // fetch it from the server
    if (result.empty()) { return SERVER_ERROR; }
    load = loadWave(ir, result.c_str(), result.size());
    if (load == SUCCESS && mUseIrCache) {
      writeFile((mIrCacheDirectory + ir.id).c_str(), result.c_str(), result.size());
    }
#endif
    return load;
  }

#ifndef SOUNDWOOFER_NO_API
  Status loadIR(std::string id, Callback callback) {
    for (auto& i : mIRlist) {
      if (id == i.id) {
        return loadIR(i, callback);
      }
    }
    return GENERIC_ERROR;
  }
#endif

  /**
   * Async version of downloadIR
   */
  Status loadIR(SWImpulse& ir, Callback callback) {
    startAsync([&]() {
      return loadIR(ir);
    }, callback);
    return ASYNC;
  }

#ifndef SOUNDWOOFER_NO_API
  SWImpulse* getIR(std::string id) {
    for (auto& i : mIRlist) {
      if (id == i.id) {
        return &i;
      }
    }
    return nullptr;
  }
#endif

  SWImpulses& getIRs() {
    return mIRlist;
  }

  SWPresets& getPresets() {
    return mPresetList;
  }

  /**
   * Will clear out all IR buffers currently in ram
   */
  void flushIRBuffers() {
    for (auto& i : mIRlist) {
      if (i.samples != nullptr) {
        for (int c = 0; c < i.channels; c++) {
          delete[] i.samples[c];
        }
        delete[] i.samples;
        i.samples = nullptr;
      }
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
   * Clears the async queue, but doesn't terminate a running task
   * Call this if for example the UI gets destroyed to be sure there are no callbacks
   * lingering which might be attached to destroyed UI elements
   */
  void clearAsyncQueue(bool doJoin = false) {
    mThreadRunning = false;
    mQueue.clear();
    if (mThread.joinable() && doJoin) {
      mThread.join();
    }
  }

  std::vector<FileInfo> scanDir(const std::string path, bool recursive = false) {
    FileInfo root;
    root.absolute = path;
    root.relative = "." + PATH_DELIMITER;
    root.name = "";
    return scanDir(root);
  }

  virtual std::vector<FileInfo> scanDir(FileInfo& root, bool recursive = false) {
#ifndef SOUNDWOOFER_CUSTOM_DIR
    std::vector<FileInfo> ret;
    struct dirent** files;
    const int count = scandir(root.absolute.c_str(), &files, nullptr, alphasort);
    if (count >= 0) {
      for (int i = 0; i < count; i++) {
        const struct dirent* ent = files[i];
        if (ent->d_name[0] != '.') {
          FileInfo info;
          info.isFolder = ent->d_type == DT_DIR;
          info.name = ent->d_name;
          info.relative = root.relative + PATH_DELIMITER + info.name;
          info.absolute = root.absolute + PATH_DELIMITER + info.name;
          root.children.push_back(info);
          if (recursive && info.isFolder) {
            scanDir(info, true);
          }
          ret.push_back(info);
        }
        free(files[i]);
      }
      free(files);
    }
    else {
      root.isFolder = false;
      assert(false);
    }
    return ret;
#endif
  }

  /**
   * Creates a directory
   * Returns success if the folder was created or already existing
   */
  static Status createFolder(const char* path) {
#ifdef _WIN32
    const bool ok = CreateDirectory(path, nullptr);
    if (!ok) {
      if (GetLastError() != ERROR_ALREADY_EXISTS) {
        return GENERIC_ERROR; // Probably permission, locked file etc
      }
    }
    return SUCCESS;
#else 
    mode_t process_mask = umask(0);
    int result_code = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
    umask(process_mask);
    return !result_code ? SUCCESS : GENERIC_ERROR;
#endif
  }

  std::string hashFile(const char* path) {
    std::ifstream fp(path);
    std::stringstream ss;

    // Unable to hash file, return an empty hash.
    if (!fp.is_open()) {
      return "";
    }

    // Hashing
    uint32_t magic = 5381;
    char c;
    while (fp.get(c)) {
      magic = ((magic << 5) + magic) + c; // magic * 33 + c
    }

    ss << std::hex << std::setw(8) << std::setfill('0') << magic;
    return ss.str();
  }

  /**
   * Singleton stuff
   */
  SoundWoofer(const SoundWoofer&) {};
  SoundWoofer& operator = (const SoundWoofer&) {};

  static SoundWoofer& instance() {
    static SoundWoofer instance;
    return instance;
  }

  virtual ~SoundWoofer() {
    flushIRBuffers();
    if (mThread.joinable()) {
      mThread.join();
    }
  }

private:
  /**
   * This will add a TaskBundle to the queue and set off
   * the thread to work on the queue if it's not already running
   */
  void startAsync(Task task, Callback callback) {
    mMutex.lock();
    mQueue.push_back({
      callback, task
    });
    if (!mThreadRunning) {
      if (mThread.joinable()) {
        mThread.join();
      }
      this->mThreadRunning = true;
      mMutex.unlock();
      mThread = std::thread([&]() {
        while(mThreadRunning) {
          mMutex.lock(); // Mutex to make sure the queue doesn't get corrupted
          TaskBundle t = *mQueue.begin();
          mQueue.erase(mQueue.begin());
          mMutex.unlock();
          const Status status = t.task();
          if (mThreadRunning) {
            // We might want to terminate here
            t.callback(status);
            mMutex.lock();
            mThreadRunning = !mQueue.empty();
            mMutex.unlock();
          }
        }
      });
    }
    else {
      mMutex.unlock();
    }
  }

  virtual SWImpulses parseIRs(std::string& data) {
    auto ret = SWImpulses();
    if (data.size() <= 2) { return ret; }
#ifndef SOUNDWOOFER_CUSTOM_JSON
    auto json = nlohmann::json::parse(data);
    for (auto i : json) {
      try {
        ret.push_back({
          i["id"],
          i["micX"], i["micY"], i["micZ"],
          i["micId"],
          i["name"],
          i["rig"],
          i["file"],
          i["description"],
          i["micPosition"],
          i["userName"],
          i["element"]
        });
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

  virtual SWRigs parseRigs(std::string& data) {
    auto ret = SWRigs();
    if (data.size() <= 2) { return ret; }
#ifndef SOUNDWOOFER_CUSTOM_JSON
    auto json = nlohmann::json::parse(data);
    for (auto i : json) {
      try {
        ret.push_back({
          i["id"],
          i["name"]
        });
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

  virtual SWPresets parsePresets(std::string& data) {
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
          i["data"]
        };
        if (preset.plugin == mPluginName) {
          ret.push_back(preset);
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

  virtual std::string encodePreset(SWPreset &preset) {
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
#endif
    return NOT_IMPLEMENTED;
  }
#endif

  /**
   * Will load the wav file to a float buffer in the SWImpulse struct
   */
  virtual Status loadWave(SWImpulse &ir, const char* waveData = nullptr, const size_t length = 0) {
#ifndef SOUNDWOOFER_CUSTOM_WAVE
    drwav wav;
    if (waveData != nullptr) {
      if (!drwav_init_memory(&wav, waveData, length, nullptr)) {
        return WAV_ERROR;
      }
    }
    else {
      const std::string path = ir.local ? (mIrDirectory + ir.file) : (mIrCacheDirectory + ir.id);
      if (!drwav_init_file(&wav, path.c_str(), nullptr)) {
        return NOT_CACHED;
      }
    }

    // interleaved buffer
    float* pSampleData = static_cast<float*>(malloc(
      static_cast<size_t>(wav.totalPCMFrameCount)* wav.channels * sizeof(float)
    ));

    ir.length = drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, pSampleData);
    if (ir.length == 0) { return WAV_ERROR; }
    ir.channels = wav.channels;
    ir.samples = new float* [ir.channels];
    for (int c = 0; c < ir.channels; c++) {
      ir.samples[c] = new float[ir.length];
    }
    for (int s = 0; s < ir.length * ir.channels; s++) {
      // slow deinterleaving
      const int channel = s % ir.channels;
      const size_t sample = s / ir.channels;
      ir.samples[channel][sample] = pSampleData[s];
    }
    free(pSampleData);
    drwav_uninit(&wav);
    return SUCCESS;
#endif
    return NOT_IMPLEMENTED;
  }

  /**
   * Not really up to spec, but this should happen on the backend anyways
   */
  static std::string generateUUID() {
    srand(time(nullptr));
    const int charLength = 10 + 26;
    const char chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    const int UUIDLength = 36;
    char out[UUIDLength];
    for (int i = 0; i < UUIDLength; i++) {
      out[i] = chars[rand() % charLength];
    }
    out[9] = out[14] = out[19] = out[24] = '-';
    std::string ret;
    ret.append(out, UUIDLength);
    return ret;
  }

  /**
   * Writes a binary file, won't append to an existing one
   */
  static Status writeFile(const char* path, const char* data, const size_t length) {
    try {
      auto file = std::fstream(path, std::ios::out | std::ios::binary);
      file.write(data, length);
      file.close();
      return SUCCESS;
    }
    catch (...) {
      return GENERIC_ERROR;
    }
  }

  static bool isWaveName(std::string& name) {
    return name.length() - name.find_last_of(".WAV") == 4
        || name.length() - name.find_last_of(".wav") == 4;
  }

  static bool isJSONName(std::string& name) {
    return name.length() - name.find_last_of(".JSON") == 5
        || name.length() - name.find_last_of(".json") == 5;
  }
};
