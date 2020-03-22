#pragma once

#ifdef _MSC_VER
  #define NOMINMAX
#endif

/**
 * This is supposed to be the headless version of the plugin
 * There's no GUI and all of the IPlug components are replaced
 */
#define GUITARD_HEADLESS

#include "../../config.h"
#include "../../thirdparty/soundwoofer/soundwoofer.h"
#include "../types/types.h"
#include "../graph/Graph.h"
#include "../nodes/RegisterNodes.h"
#include "../misc/MessageBus.h"
#include "../parameter/ParameterManager.h"

namespace guitard {
  /**
   *  Simple object wrapping a graph parameter manager and bus
   */
  class GuitarDHeadless {
    MessageBus::Bus mBus;
    ParameterManager mParamManager;
    Graph mGraph;
    bool mReady = false;
  public:
    GuitarDHeadless() : mParamManager(&mBus), mGraph(&mParamManager) {
      String homeDir;
#ifdef unix
      homeDir = getenv("HOME");
#elif defined(_WIN32)
      homeDir = getenv("HOMEDRIVE");
      homeDir.append(getenv("HOMEPATH"));
#endif
      printf("\n%s\n", homeDir.c_str());
      soundwoofer::setup::setPluginName("GuitarD");
      HOME_PATH = homeDir;
      soundwoofer::setup::setHomeDirectory(homeDir.c_str());
    }

    /**
     * Needs to be called before processing can start to set sample rate and channel config
     */
    void setConfig(int samplerate, int outChannels, int inChannels) {
      if (samplerate > 0 && outChannels > 0 && inChannels > 0) {
        mGraph.OnReset(samplerate, outChannels, inChannels);
        mReady = true;
      }
      else {
        mReady = false;
      }
    }

    void process(sample** in, sample** out, int samples) {
      if (mReady) {
        mGraph.ProcessBlock(in, out, samples);
      }
    }

    /**
     * Resets the plugin (kills reverb tails etc)
     */
    void reset() {
      mGraph.OnTransport();
    }

    /**
     * Takes a value from 0 to 1 to control the parameter
     */
    void setParam(int paramIndex, sample value) {
      ParameterCoupling* couple = mParamManager.getCoupling(paramIndex);
      if (couple != nullptr) {
        couple->setFromNormalized(value);
      }
    }

    /**
     * Provide a json to load, make sure it's null terminated
     * Will block the thread until it's loaded and skip processing
     */
    void load(const char* data) {
      mGraph.deserialize(data);
    }
  };
}