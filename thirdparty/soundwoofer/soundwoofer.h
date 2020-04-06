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
    void setPluginName(const std::string& name);

    void setPluginVersion(const std::string& version);

    /**
     * Sets the location for data like caches etc
     */
    Status setHomeDirectory(std::string path);
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
    void flushBuffers();

    /**
     * Will clear out the disk cached IRs
     */
#ifndef SOUNDWOOFER_NO_API
    void clearIRCache();
#endif

    /**
     * Will add a generic Mic, amp, factory presets and factory IRs
     */
    void resetAll();

    /**
     * Gets a list of all the Impulse responses from the server and the local folder
     * Will also call clearCachedIRs()
     */
    Status list(bool reset);

    /**
     * Download and decode a specific IR
     * If it's local IR or cached, it will skip the download
     * The decoded IR will be in SWImpulse::samples
     * Will block, use a lambda as a callback for async
     */
    Status load(SWImpulseShared ir, size_t sampleRate, bool normalize);

    Status loadUnknown(SWImpulseShared* ir, size_t sampleRate, bool normalize);

    SWImpulseShared createGeneric(
      std::string name, float** samples, size_t length, size_t channels, size_t sampleRate, Source source
    );

    SWRigs getRig();
  }

  namespace preset {

    /**
     * Fetches presets from the server and the local folder
     */
    Status list();

    SWPresets get();

    Status load(SWPresetShared preset);

    Status save(const SWPresetShared preset);

    /**
     * Sends the preset to the server
     */
#ifndef SOUNDWOOFER_NO_API
    Status send(const SWPresetShared preset);
#endif
  }
}

#ifndef SOUNDWOOFER_NO_ASYNC
  #include "./soundwooferAsync.h"
#endif

#ifdef SOUNDWOOFER_IMPL
  #include "./soundwooferImpl.h"
#endif