#pragma once

#ifdef _MSC_VER
  #define NOMINMAX
#endif

#define SAMPLE_TYPE_FLOAT
#define GUITARD_SSE

/**
 * This is supposed to be the headless version of the plugin
 * There's no GUI and all of the IPlug components are replaced
 */
#define GUITARD_HEADLESS

#include "../../../config.h" // This is the iplug config
#include "../../types/GTypes.h"
namespace guitard {
  /**
   *  Simple object wrapping a graph parameter manager and bus
   */
  class GuitarDHeadless {
    ParameterManager* mParamManager = nullptr;
    Graph* mGraph = nullptr;
    bool mReady = false;
  public:
    GuitarDHeadless();

    ~GuitarDHeadless();

    /**
     * Needs to be called before processing can start to set sample rate and channel config
     */
    void setConfig(int samplerate, int outChannels, int inChannels);

    void process(sample** in, sample** out, int samples);

    /**
     * Resets the plugin (kills reverb tails etc)
     */
    void reset();

    /**
     * Takes a value from 0 to 1 to control the parameter
     */
    void setParam(int paramIndex, sample value);

    /**
     * Provide a json to load, make sure it's null terminated
     * Will block the thread until it's loaded and skip processing
     */
    void load(const char* data);
  };
}