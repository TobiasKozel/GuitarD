#pragma once

#ifndef SOUNDWOOFER_CUSTOM_JSON
#include "json.hpp"
#endif

#ifndef SOUNDWOOFER_CUSTOM_HTTP
#include "httplib.h"
#endif

#ifndef SOUNDWOOFER_CUSTOM_WAVE
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#endif

class SoundWoofer {
public:
  struct SWImpulse {
    std::string id;
    int micX;
    int micY;
    int micZ;
    std::string micId;
    std::string name;
    std::string rig;
    std::string file;
    std::string description;
    int micPosition;
    std::string userName;
    std::string element;
    int channels = 0;
    int length = 0;
    float** samples = nullptr;
  };

  struct SWMic {
    std::string id;
    // TODO Mic object doesn't exist on the backend
  };

  struct SWRig {
    std::string id;
    std::string name;
  };

  typedef std::vector<SWImpulse> SWImpulses;
  typedef std::vector<SWRig> SWRigs;

private:
  const std::string BACKEND_URL = "svenssj.tech";
  const int BACKEND_PORT = 5000;

  SWImpulses mIRlist;
  SWRigs mCabList;

  SoundWoofer() { }
public:

  /**
   * Gets a list of all the Impulse responses from the server
   */
  void fetchIRs() {
    std::string data = httpGet("/Impulse");
    mIRlist = parseIRs(data);
    data = httpGet("/Rig");
    mCabList = parseRigs(data);
    // TODO assign them to the cabs an mics
  }

  void downloadIR(SWImpulse &ir) {
    if (ir.samples != nullptr) {
      return;
    }
    std::string result = httpGet("/File/Download/" + ir.file);
    loadWave(ir, result.c_str(), result.size());
  }

  SWImpulses& getIRs() {
    return mIRlist;
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

  ~SoundWoofer() {
    for (auto i : mIRlist) {
      if (i.samples != nullptr) {
        for (int c = 0; c < i.channels; c++) {
          delete[] i.samples[c];
        }
        delete[] i.samples;
      }
    }
  }
private:
  
  virtual SWImpulses parseIRs(std::string &data) {
    auto ret = SWImpulses();
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
        int test = 0;
        //assert(false, "Error parsing IR");
      }
    }
#else
    assert(false, "You need to override this function if you want to use a different json parser!");
#endif
    return ret;
  }

  virtual SWRigs parseRigs(std::string &data) {
    auto ret = SWRigs();
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
        assert(false, "Error parsing Cab");
      }
    }
#else
    assert(false, "You need to override this function if you want to use a different json parser!");
#endif
    return ret;
  }

  virtual std::string httpGet(std::string endpoint) {
#ifndef SOUNDWOOFER_CUSTOM_HTTP
    httplib::Client cli(BACKEND_URL, BACKEND_PORT);
    auto res = cli.Get(endpoint.c_str());
    if (res && res->status == 200) {
      return res->body;
    }
#else
    assert(false, "You need to override this function if you want to use a different http lib!");
#endif
    return "";
  }

  virtual void loadWave(SWImpulse &ir, const char* waveData, const size_t length) {
#ifndef SOUNDWOOFER_CUSTOM_WAVE
    drwav wav;
    if (!drwav_init_memory(&wav, waveData, length, nullptr)) {
      return;
    }

    float* pSampleData = static_cast<float*>(malloc(
      static_cast<size_t>(wav.totalPCMFrameCount)* wav.channels * sizeof(float)
    ));

    ir.length = drwav_read_pcm_frames_f32(&wav, wav.totalPCMFrameCount, pSampleData);
    ir.channels = wav.channels;
    ir.samples = new float* [ir.channels];
    for (int c = 0; c < ir.channels; c++) {
      ir.samples[c] = new float[ir.length];
    }
    for (int s = 0; s < ir.length * ir.channels; s++) {
      const int channel = s % ir.channels;
      const size_t sample = s / ir.channels;
      ir.samples[channel][sample] = pSampleData[s];
    }
    free(pSampleData);
    drwav_uninit(&wav);
  }
#endif
};