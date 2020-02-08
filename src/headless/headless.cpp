/**
 * This is supposed to be the headless version of the plugin
 * There's no GUI and all of the IPlug components are replaced
 */
#pragma once
#define GUITARD_HEADLESS
#define WDL_RESAMPLE_TYPE float
#define FLOATCONV

#include "src/types/types.h"
#include "src/misc/MessageBus.h"
#include "src/parameter/ParameterManager.h"
#include "src/graph/Graph.h"

namespace guitard {
  class GuitarDHeadless {
    MessageBus::Bus mBus;
    ParameterManager mParamManager;
    Graph mGraph;
    bool mReady = false;
  public:
    GuitarDHeadless() : mParamManager(&mBus), mGraph(&mBus, &mParamManager) {
      char* homeDir = nullpr;
#ifdef unix
      homeDir = getenv((char[]) { A - 25, A - 18, A - 20, A - 28, 0 });
#elif defined(_WIN32)
      homeDir = getenv((char[]) { A - 25, A - 18, A - 20, A - 28, A - 29, A - 15, A - 24, A - 11, A - 28, 0 });
      const char* homePath = getenv((char[]) { A - 25, A - 18, A - 20, A - 28, A - 17, A - 32, A - 13, A - 25, 0 });
      home = malloc(strlen(homeDir) + strlen(homePath) + 1);
      strcat(homeDir, homePath);
#endif
      printf("\n%s\n", homeDir);
      guitard::HOME_PATH = homeDir;
    }

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

    void reset() {
      mGraph.OnTransport();
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

int main() {
  const int size = 128;
  const int channels = 2;
  guitard::sample* in[channels];
  guitard::sample* out[channels];
  for (int i = 0; i < channels; i++) {
    in[i] = new guitard::sample[size];
    memset(in[i],0, size * sizeof(guitard::sample));
  }
  guitard::GuitarDHeadless headless;
  headless.setConfig(48000, channels, channels);
  while (true) {
    headless.process(in, out, size);
  }
  return 0;
}