
#include "./GHeadlessUnit.h"
#include "../../Dependencies/Extras/nlohmann/json.hpp"
#include "../../main/Graph.h"
#include "../../nodes/RegisterNodes.h"
#include "../../main/parameter/ParameterManager.h"



namespace guitard {
  GuitarDHeadless::GuitarDHeadless() {
    mParamManager = new ParameterManager();
    mGraph = new Graph();
    mGraph->setParameterManager(mParamManager);
  }

  GuitarDHeadless::~GuitarDHeadless() {
    delete mGraph;
    delete mParamManager;
  }

  /**
   * Needs to be called before processing can start to set sample rate and channel config
   */
  void GuitarDHeadless::setConfig(int samplerate, int outChannels, int inChannels) {
    if (samplerate > 0 && outChannels > 0 && inChannels > 0) {
      mGraph->OnReset(samplerate, outChannels, inChannels);
      mReady = true;
    }
    else {
      mReady = false;
    }
  }

  void GuitarDHeadless::process(const sample** in, sample** out, int samples) {
    if (mReady) {
      mGraph->ProcessBlock(const_cast<sample**>(in), out, samples);
    }
  }

  /**
   * Resets the plugin (kills reverb tails etc)
   */
  void GuitarDHeadless::reset() {
    if (mReady) {
      mGraph->OnTransport();
    }
  }

  /**
   * Takes a value from 0 to 1 to control the parameter
   */
  void GuitarDHeadless::setParam(int paramIndex, sample value) {
    ParameterCoupling* couple = mParamManager->getCoupling(paramIndex);
    if (couple != nullptr) {
      couple->setFromNormalized(value);
    }
  }

  /**
   * Provide a json to load, make sure it's null terminated
   * Will block the thread until it's loaded and skip processing
   */
  void GuitarDHeadless::load(const char* data) {
    mGraph->deserialize(data);
  }
}