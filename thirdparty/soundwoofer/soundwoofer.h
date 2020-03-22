/**
 * Soundwoofer is a online service which provides Impulse responses
 * They plan on developing a api to access them from plugins
 * This is kind of a barebones version of that
 * It also can fetch Presets
 */

#pragma once

// #define SOUNDWOOFER_NO_API

#include "./soundwooferTypes.h"
#include "./soundwooferFile.h"
#include "./soundwooferWave.h"
#include "./soundwooferSerialize.h"

#ifndef SOUNDWOOFER_NO_API
  #include "./soundwooferHttp.h"
#endif

namespace soundwoofer {
  namespace setup {
    /**
     * Set this first, it's needed for caching and for online presets
     */
    void setPluginName(const std::string& name) {
      state::pluginName = name;
      state::cacheIRs = true;
      state::cachePresets = true;
    }

    void setPluginVersion(const std::string& version) {
      state::pluginVersion = version;
    }

    /**
     * Sets the location for data like caches etc
     */
    Status setHomeDirectory(std::string path) {
      if (state::pluginName.empty()) { return PLUGIN_NAME_NOT_SET; }
      path += file::PATH_DELIMITER + state::pluginName + file::PATH_DELIMITER;
      state::homeDirectory = path;
      state::presetDirectory = path + "presets" + file::PATH_DELIMITER;
      state::irDirectory = path + "irs" + file::PATH_DELIMITER;
      state::presetCacheDirectory = path + "preset_cache" + file::PATH_DELIMITER;
      state::irCacheDirectory = path + "ir_cache" + file::PATH_DELIMITER;
      bool ok = true;
      ok = file::createFolder(path.c_str()) != SUCCESS ? false : ok;
#ifndef SOUNDWOOFER_NO_API
      ok = file::createFolder(state::presetCacheDirectory.c_str()) != SUCCESS ? false : ok;
      ok = file::createFolder(state::irCacheDirectory.c_str()) != SUCCESS ? false : ok;
#endif
      ok = file::createFolder(state::presetDirectory.c_str()) != SUCCESS ? false : ok;
      ok = file::createFolder(state::irDirectory.c_str()) != SUCCESS ? false : ok;
      return ok ? SUCCESS : GENERIC_ERROR;
    }
  }

  namespace ir {
    Status list(bool reset = true);

    Status load(SWImpulseShared ir, size_t sampleRate = 0, bool normalize = true);

    /**
     * Load from an unknown source, like when loading up the project
     * before list() was called
     */
    Status loadUnknown(SWImpulseShared* ir, size_t sampleRate = 0, bool normalize = true);

    /**
     * Create IRs which live outside the SoundWoofer domain
     */
    SWImpulseShared createGeneric(
      std::string name, float** samples, size_t length, size_t channels = 1,
      size_t sampleRate = 48000, Source source = EMBEDDED_SRC
    );

    /**
     * Will clear out all IR buffers currently in ram
     */ 
    void flushBuffers() {
      for (auto& i : state::irList) {
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
     * Will add a generic Mic, amp, factory presets and factory IRs
     */
    void resetAll() {
      flushBuffers();
      for (auto& i : state::componentList) { i->managed = false; }
      state::componentList.clear();
      state::componentListCached = false;
      for (auto& i : state::rigList) { i->managed = false; }
      state::rigList.clear();
      state::rigListCached = false;
      for (auto& i : state::irList) { i->managed = false; }
      state::irList.clear();
      state::rigListCached = false;
      state::GenericRig->impulses.clear();
      state::GenericRig->components.clear();
      state::GenericRig->impulses.clear();
      state::componentList.push_back(state::GenericComponent);
      state::componentList.push_back(state::GenericMicrophone);
      state::rigList.push_back(state::GenericRig);
      state::GenericRig->components.push_back(state::GenericComponent);
      state::GenericRig->components.push_back(state::GenericMicrophone);
    }

    

    /**
     * Gets a list of all the Impulse responses from the server and the local folder
     * Will also call clearCachedIRs()
     */
    Status list(bool reset) {
      if (reset) {
        resetAll();
      }

      // This will cunstruct a generic IR with no cab or mic as a parent
      auto addToUncategorized = [&](file::FileInfo& info) {
        if (!file::isWaveName(info.name)) { return; }
        SWImpulseShared ir(
          new SWImpulse {
            "CHECKSUM", info.name, state::GenericMicrophone->id,
            state::GenericRig->id, info.relative, USER_SRC
          }
        );
        state::GenericRig->impulses.push_back(ir);
      };

      if (!state::irDirectory.empty()) { // Gather the user IRs in the IR directory
        auto cabLevel = file::scanDir(state::irDirectory);
        for (auto i : cabLevel) {
          if (i.isFolder) {
            SWRigShared rig(new SWRig{ i.name, i.name, USER_SRC });
            auto micLevel = file::scanDir(i);
            for (auto j : micLevel) {
              SWComponentShared existingMic;
              for (auto& m : state::componentList) {
                if (m->name == j.name) {
                  existingMic = m;
                  break;
                }
              }
              SWComponentShared mic;
              if (existingMic == nullptr) { // Make a new one
                mic = SWComponentShared(new SWComponent{ j.name, j.name, TypeMicrophone, USER_SRC });
              }
              else {
                mic = existingMic;
              }
              
              rig->microphones.push_back(mic);
              if (existingMic == nullptr) { state::componentList.push_back(mic); }  // Only register it globally once
              if (j.isFolder) {
                auto posLevel = file::scanDir(j);
                for (auto k : posLevel) {
                  if (!k.isFolder && file::isWaveName(k.name)) {
                    SWImpulseShared ir(new SWImpulse{ "CHECKSUM", k.name, mic->id, rig->id, k.relative, USER_SRC });
                    state::irList.push_back(ir); // GLOBAL
                    rig->impulses.push_back(ir);
                  }
                }
              }
              else { addToUncategorized(j); } // At mic level
            }
            state::rigList.push_back(rig); // GLOBAL
          }
          else { addToUncategorized(i); } // At Cabinet level
        }
      }

#ifndef SOUNDWOOFER_NO_API
      std::string data;
      data = http::get("/Impulse");
      if (data.empty()) {
        return SERVER_ERROR;
      }
      SWImpulses swIrs = parse::irs(data);
      //for (auto& i : swIrs) {
      //  load(i, 48000);
      //}
      state::irList.insert(state::irList.end(), swIrs.begin(), swIrs.end());
      data = http::get("/Component");
      if (data.empty()) {
        return SERVER_ERROR;
      }
      SWComponents swComps = parse::components(data);
      state::componentList.insert(state::componentList.end(), swComps.begin(), swComps.end());
      data = http::get("/Rig");
      if (data.empty()) {
        return SERVER_ERROR;
      }
      SWRigs swRigs = parse::rigs(data);
      for (auto& rig : swRigs) {
        state::rigList.push_back(rig);
        for (auto& ir : state::irList) {
          if (rig->id == ir->rig) {
            rig->impulses.push_back(ir);
            for (auto& mic : state::componentList) {
              if (mic->id == ir->micId) {
                rig->microphones.push_back(mic);
                mic->type = TypeMicrophone;
                break;
              }
            }
          }
        }
      }
#endif
      return SUCCESS;
      // TODO assign them to the cabs an mics
    }

    /**
     * Download and decode a specific IR
     * If it's local IR or cached, it will skip the download
     * The decoded IR will be in SWImpulse::samples
     * Will block, use a lambda as a callback for async
     */
    Status load(SWImpulseShared ir, size_t sampleRate, bool normalize) {
      if (ir == nullptr || (ir->id.empty() && ir->file.empty())) { return GENERIC_ERROR; }
      if (ir->samples != nullptr) {
        if (normalize) {
          wave::normalizeIR(ir);
        }
        return SUCCESS; // Already in ram
      }

      Status ret = GENERIC_ERROR;
      // Try loading the wave from disk first
      std::string path;
      if (ir->source == USER_SRC) { path = state::irDirectory + ir->file; }
      if (ir->source == USER_SRC_ABSOLUTE) { path = ir->file; }
      if (ir->source == SOUNDWOOFER_SRC) { path = state::irCacheDirectory + ir->file; }
      ret = wave::loadWaveFile(ir, path, sampleRate, normalize);
      if (ret == SUCCESS) { return SUCCESS; } // Loaded from disk
      if (ir->source != SOUNDWOOFER_SRC) { return UNKNOWN_IR; } // No file and also not a online IR
#ifndef SOUNDWOOFER_NO_API
    // Go look online
      const std::string result = http::get("/File/Download/" + ir->file);
      if (result.empty()) { return SERVER_ERROR; }
      ret = wave::loadWaveMemory(ir, result.c_str(), result.size(), sampleRate, normalize);
      if (ret == SUCCESS && state::cacheIRs) {
        // Cache the file
        file::writeFile((state::irCacheDirectory + ir->file).c_str(), result.c_str(), result.size());
      }
#endif
      return ret;
    }

    Status loadUnknown(SWImpulseShared* ir, size_t sampleRate, bool normalize) {
      for (auto& i : state::irList) {
        if (i->file == (*ir)->file) {
          (*ir) = i; // Already known IR
        }
      }
      if (file::isUUID((*ir)->file)) {
        (*ir)->source = SOUNDWOOFER_SRC; // Soundwoofer ir, not yet known
      }
      if (file::isRelative((*ir)->file) && file::isWaveName((*ir)->file)) {
        (*ir)->source = USER_SRC;
      }
      return load((*ir), sampleRate, normalize);
    }

    SWImpulseShared createGeneric(
      std::string name, float** samples, size_t length, size_t channels, size_t sampleRate, Source source
    ) {
      SWImpulseShared ret(new SWImpulse());
      ret->name = name;
      ret->id = "generic";
      ret->samples = samples;
      ret->length = length;
      ret->channels = channels;
      ret->sampleRate = sampleRate;
      ret->source = source;
      return ret;
    }

    SWRigs getRig() {
      return state::rigList;
    }
  }

  namespace preset {

    /**
     * Fetches presets from the server and the local folder
     */
    Status list() {
      state::presetList.clear();

      for (auto i : state::factoryPresetList) { // Gather factory presets
        state::presetList.push_back(std::make_shared<SWPreset>(i));
      }

      if (!state::presetDirectory.empty()) { // Gather user presets
        auto files = file::scanDir(state::presetDirectory);
        for (auto i : files) {
          if (!i.isFolder && file::isJSONName(i.name)) {
            SWPresetShared preset(new SWPreset{ i.name, i.name, state::pluginName, USER_SRC });
            state::presetList.push_back(preset);
          }
        }
      }
#ifndef SOUNDWOOFER_NO_API
      // Gather soundwoofer presets
      if (state::pluginName.empty()) { return PLUGIN_NAME_NOT_SET; }
      std::string data = http::get("/Preset");
      if (data.empty()) { return SERVER_ERROR; }
      SWPresets online = parse::presets(data);
      state::presetList.insert(state::presetList.end(), online.begin(), online.end());
#endif
      return SUCCESS;
    }

    SWPresets get() {
      return state::presetList;
    }

    Status load(SWPresetShared preset) {
      if (!preset->data.empty()) { return SUCCESS; }
#ifndef SOUNDWOOFER_NO_API
      if (preset->source == SOUNDWOOFER_SRC) { // online
        const std::string result = http::get("/Preset/" + preset->id);
        if (result.empty()) { return SERVER_ERROR; }
        const auto dl = parse::presets(result);
        if (dl.size() != 1) { return SERVER_ERROR; }
        preset->data = dl.at(0)->data;
        return SUCCESS;
      }
#endif
      // offline
      std::ifstream file(state::presetDirectory + file::PATH_DELIMITER + preset->name);
      // TODO binary presets
      const std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      preset->data = str;
      return SUCCESS;
    }

    Status save(const SWPresetShared preset) {
      // TODO binary presets
      Status status = file::writeFile(
        (state::presetDirectory + file::PATH_DELIMITER + preset->name).c_str(),
        preset->data.c_str(), preset->data.size()
      );
      if (status == SUCCESS) {
        // state::presetList.push_back(preset);
      }
      return status;
    }

#ifndef SOUNDWOOFER_NO_API
    /**
     * Sends the preset to the server
     */
    Status send(const SWPresetShared preset) {
      if (state::pluginName.empty()) { return PLUGIN_NAME_NOT_SET; }

      const std::string serialized = encode::preset(preset);
      if (serialized.empty()) { return JSON_ENCODE_ERROR; }
      return http::post("/Preset", serialized.c_str(), serialized.size());
    }
#endif
  }
}

#ifndef SOUNDWOOFER_NO_ASYNC
  #include "./soundwooferAsync.h"
#endif