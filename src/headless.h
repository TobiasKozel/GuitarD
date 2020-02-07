/**
 * This is supposed to be the headless version of the plugin
 * There's no GUI and all of the IPlug components are replaced
 */
#pragma once
#define GUITARD_HEADLESS
#include "misc/MessageBus.h"
#include "parameter/ParameterManager.h"
#include "graph/Graph.h"

namespace guitard {
  class GuitarDHeadless {
    MessageBus::Bus mBus;
    ParameterManager mParamManager;
    Graph mGraph;

  public:
    GuitarDHeadless() : mParamManager(&mBus), mGraph(&mBus, &mParamManager) {

    }


  };
}