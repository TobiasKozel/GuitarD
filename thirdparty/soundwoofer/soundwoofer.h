#pragma once

#ifndef SOUNDWOOFER_CUSTOM_JSON
  #include "./dependencies/json.hpp"
#endif

#define SOUNDWOOFER_NO_API

#ifndef SOUNDWOOFER_NO_API
  #ifndef SOUNDWOOFER_CUSTOM_HTTP
    // #define CPPHTTPLIB_OPENSSL_SUPPORT
    #include "./dependencies/httplib.h"
  #endif
#endif

#ifndef SOUNDWOOFER_CUSTOM_WAVE
  #define DR_WAV_IMPLEMENTATION
  #include "./dependencies/dr_wav.h"
#endif

#ifndef SOUNDWOOFER_CUSTOM_DIR
  #ifdef _WIN32
    #include "./dependencies/dirent.h"
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

  enum Source {
    SOUNDWOOFER_SRC = 0,
    USER_SRC,
    FACTORY_SRC
  };

  /**
   * Soundwoofer structs
   */

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
  typedef std::shared_ptr<SWPreset> SWPresetsShared;
  typedef std::vector<SWPresetsShared> SWPresets;

  struct SWImpulse {
    std::string id;
    std::string name;
    std::string micId; // The ID of the mic used to record this
    std::string rig; // The ID of the SWRig used to record this, not a reference to precent circular deps
    std::string file; // This is either a uuid or a path
    Source source = SOUNDWOOFER_SRC;
    int micX = 0;
    int micY = 0;
    int micZ = 0;
    std::string description;
    int micPosition;
    std::string userName;
    std::string element;
    int channels = 0;
    int length = 0;
    float** samples = nullptr;
    int sampleRate = 0;
    void clearSamples() {
      if (samples == nullptr || source == FACTORY_SRC) { return; }
      for (int i = 0; i < channels; i++) {
        delete[] samples[i];
      }
      delete[] samples;
      samples = nullptr;
      channels = length = 0;
    }
    ~SWImpulse() {
      clearSamples();
    }
  };

  const std::string TypeMicrophone = "Microphone";
  const std::string TypeCabinet = "Cabinet";

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
    int year = 0;
    std::string url;
    std::string userName;
    std::string brand;
    std::string model;
    std::string baseComponentDescription;

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
    std::string username;
    std::string description;
    SWComponents components;
    SWComponents microphones;
    SWImpulses impulses;
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
    std::vector<std::string> tags;
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

  /**
   * Callback for async operations which provides a status code
   */
  typedef std::function<void(Status)> Callback;


private:
  /**
   * Generic Objects to categorize IRs with no parent
   */
  SWComponentShared GenericComponent;
  SWComponentShared GenericMicrophone;
  SWRigShared GenericRig;

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
  std::vector<SWPreset> mFactoryPresetList;

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
    GenericComponent = std::make_shared<SWComponent>(SWComponent {
      "Generic Component", "Generic Component", TypeCabinet, USER_SRC
    });
    GenericMicrophone = std::make_shared<SWComponent>(SWComponent {
      "Generic Microphone", "Generic Microphone", TypeMicrophone, USER_SRC
    });
    GenericRig = std::make_shared<SWRig>(SWRig {
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
    resetIRs();

    // This will cunstruct a generic IR with no cab or mic as a parent
    auto addToUncategorized = [&](FileInfo& info) {
      if (!isWaveName(info.name)) { return; }
      SWImpulseShared ir(new SWImpulse{ "CHECKSUM", info.name, GenericMicrophone->id, GenericRig->id, info.relative, USER_SRC });
      GenericRig->impulses.push_back(ir);
    };

    if (!mIrDirectory.empty()) { // Gather the user IRs in the IR directory
      auto cabLevel = scanDir(mIrDirectory);
      for (auto i : cabLevel) {
        if (i.isFolder) {
          SWRigShared rig (new SWRig { i.name, i.name, USER_SRC });
          auto micLevel = scanDir(i);
          for (auto j : micLevel) {
            bool micExists = false;
            for (auto& m : mComponentList) {
              if (m->name == j.name) {
                micExists = true;
                break;
              }
            }
            SWComponentShared mic(new SWComponent { j.name, j.name, TypeMicrophone, USER_SRC });
            rig->microphones.push_back(mic);
            if (!micExists) { mComponentList.push_back(mic); }  // GLOBAL
            if (j.isFolder) {
              auto posLevel = scanDir(j);
              for (auto k : posLevel) {
                if (!k.isFolder && isWaveName(k.name)) {
                  SWImpulseShared ir(new SWImpulse{ "CHECKSUM", k.name, mic->id, rig->id, k.relative, USER_SRC });
                  mIRlist.push_back(ir); // GLOBAL
                  rig->impulses.push_back(ir);
                }
              }
            } else { addToUncategorized(j); } // At mic level
          }
          mRigList.push_back(rig); // GLOBAL
        } else { addToUncategorized(i); } // At Cabinet level
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
    
    for (auto i : mFactoryPresetList) { // Gather factory presets
      mPresetList.push_back(std::make_shared<SWPreset>(i));
    }

    if (!mPresetDirectory.empty()) { // Gather user presets
      auto files = scanDir(mPresetDirectory);
      for (auto i : files) {
        if (!i.isFolder && isJSONName(i.name)) {
          SWPresetsShared preset(new SWPreset { i.name, i.name, mPluginName, USER_SRC });
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

  Status listPresets(const Callback callback) {
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
  Status loadIR(std::string fileId) {
    for (auto &i : mIRlist) {
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
    if (unknownIR) { return UNKNOWN_IR; }
#ifndef SOUNDWOOFER_NO_API
    if (ir->source == SOUNDWOOFER_SRC) {
      const std::string result = httpGet("/File/Download/" + ir->file); // fetch it from the server
      if (result.empty()) { return SERVER_ERROR; }
      load = loadWave(ir, result.c_str(), result.size());
      if (load == SUCCESS && mCacheIRs) { // Cache the file
        writeFile((mIrCacheDirectory + ir->id).c_str(), result.c_str(), result.size());
      }
    }
#endif
    return load;
  }

#ifndef SOUNDWOOFER_NO_API
  Status loadIR(std::string fileId, Callback callback) {
    for (auto& i : mIRlist) {
      if (fileId == i->file) {
        return loadIR(i, callback);
      }
    }
    return GENERIC_ERROR;
  }
#endif

  /**
   * Async version of downloadIR
   */
  Status loadIR(SWImpulseShared& ir, Callback callback) {
    startAsync([&]() {
      return loadIR(ir);
    }, callback);
    return ASYNC;
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
   * Clears the async queue, but doesn't terminate a running task
   * Call this if for example the UI gets destroyed to be sure there are no callbacks
   * lingering which might be attached to destroyed UI elements
   */
  void clearAsyncQueue(const bool doJoin = false) {
    mThreadRunning = false;
    mQueue.clear();
    if (mThread.joinable() && doJoin) {
      mThread.join();
    }
  }

  std::vector<FileInfo> scanDir(const std::string path, const bool recursive = false) {
    FileInfo root;
    root.absolute = path;
    root.relative = "." + PATH_DELIMITER;
    root.name = "";
    return scanDir(root, recursive);
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

  /**
   * Simple file hashing from
   * https://gist.github.com/nitrix/34196ff0c93fdfb01d51
   */
  static std::string hashFile(const std::string path) {
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
  SoundWoofer(const SoundWoofer&) = delete;
  SoundWoofer& operator = (const SoundWoofer&) = delete;

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
        ret.push_back(
          SWImpulseShared(new SWImpulse {
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

  virtual SWRigs parseRigs(std::string& data) {
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
    else { // Either look in the user IR folder or the IR cache folder
      const std::string path = ir->source == USER_SRC ? (mIrDirectory + ir->file) : (mIrCacheDirectory + ir->id);
      if (!drwav_init_file(&wav, path.c_str(), nullptr)) {
        return NOT_CACHED; // This means we'll need to go online and get the IR
      }
      if (ir->source == USER_SRC) { // Hash the file so we can go look for it if the file is missing on load
        ir->id = hashFile(mIrDirectory + ir->file);
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

  static Status deleteFile(const char* path) {
    return std::remove(path) == 0 ? SUCCESS : GENERIC_ERROR;
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
