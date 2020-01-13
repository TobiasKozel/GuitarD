#pragma once

#include "httplib.h"
#include <future>


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

    void download() {

    }
  };

private:
  bool mHasIRlist = false;
  bool mHasPresetList = false;
  std::function<void(bool)> mIRfetchCallback = nullptr;
  std::vector<SWImpulse> mIRlist;

  SoundWoofer() {
  }
public:

  bool fetchIRs(std::function<void (bool)> callback = nullptr) {
    if (mHasIRlist) {
      if (callback != nullptr) {
        callback(true);
      }
      return true;
    }

    if (callback != nullptr) {
      mIRfetchCallback = callback;
      std::async(std::launch::async, &SoundWoofer::doFetchIRs, this);
      return false;
    }
    doFetchIRs();
    return mHasIRlist;
  }

  SoundWoofer(const SoundWoofer&) = delete;
  SoundWoofer& operator = (const SoundWoofer&) = delete;

  static SoundWoofer& instance() {
    static SoundWoofer instance;
    return instance;
  }

private:
  void doFetchIRs() {
    httplib::Client cli("svenssj.tech", 5000);
    auto res = cli.Get("/Impulse");
    if (res && res->status == 200) {
      auto ret = std::vector<SWImpulse>();
      auto json = nlohmann::json::parse(res->body);
      for (auto i : json) {
        ret.push_back({
          json["id"],
          json["micX"], json["micY"], json["micZ"],
          json["micId"],
          json["name"],
          json["rig"],
          json["file"],
          json["description"],
          json["micPosition"],
          json["userName"],
          json["element"]
          });
      }
      this->mIRlist = ret;
      this->mHasIRlist = true;
    }
    this->mHasIRlist = false;
    if (mIRfetchCallback != nullptr) {
      mIRfetchCallback(this->mHasIRlist);
      mIRfetchCallback = nullptr;
    }
  }
};