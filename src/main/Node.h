#pragma once

#include "../types/GTypes.h"
#include "../types/GStructs.h"
#include "../types/GOversampler.h"
#include "../GConfig.h"

#include "./NodeSocket.h"
#include "./parameter/ParameterCoupling.h"
#include "./parameter/MeterCoupling.h"

#include "./factory/NodeList.h"

#include "../../thirdparty/soundwoofer/dependencies/json.hpp"

namespace guitard {
  /**
   * Virtual class which all nodes will derive from
   * It's the DSP part of the node
   */
  class Node {
  protected:
    Oversampler* mOverSampler = nullptr;
  public: // Everything is public since it most of it needs to be accessible from the graph and the NodeUi
    sample mOverSamplingFactor = 1.0; // The oversampling factor bound to the control
    int mOverSamplingIndex = -1;

    int mSampleRate = 0;
    int mLastBlockSize = 0;
    // This size will be used to allocate the dsp buffer, the actual samples per block can be lowe
    int mMaxBlockSize = 0; 
    int mChannelCount = 0;

    sample mByPassed = 0;
    int mByPassedIndex = -1;
    sample mStereo = 1;

    int mParameterCount = 0;
    ParameterCoupling mParameters[GUITARD_MAX_NODE_PARAMETERS];
    int mMeterCount = 0;
    MeterCoupling mMeters[GUITARD_MAX_NODE_METERS];

    int mInputCount = 0;
    NodeSocket mSocketsIn[GUITARD_MAX_NODE_SOCKETS];
    int mOutputCount = 0;
    NodeSocket mSocketsOut[GUITARD_MAX_NODE_SOCKETS];

    /**
     * Nodes which this one depends on an need to be processed first
     * Every socket can be a dependency and every parameter for automation
     */
    Node* mDependencies[GUITARD_MAX_NODE_SOCKETS + GUITARD_MAX_NODE_PARAMETERS] = { nullptr };
    int mDependencyCount = 0;

    Coord2D mPos = { 0, 0 }; // Position on the canvas in pixels
    Coord2D mDimensions = { 250, 200 }; // Size in Pixels

    NodeList::NodeInfo* mInfo = nullptr;

    /**
     * Will be called from the NodeList factory directly after the object is constructed
     */
    void setNodeInfo(NodeList::NodeInfo* info) {
      assert(mInfo == nullptr);
      mInfo = info;
    }

    /**
     * This is basically a delayed constructor with the only disadvantage: derived methods have to have the same parameters
     * The derived class will call this with the desired parameters, so they can ignore pInputs, pOutputs and pOutputs
     * @param pSamplerate Sampling rate the node will operate at
     * @param pMaxBuffer The maximum Buffer size the ProcessBlock will be called
     * @param pInputs Number of Input sockets the node should have
     * @param pOutputs Number of Output sockets the node should have, will create buffer for them
     * @param pChannels Channel count per socket, has to be 2 for now
     */
    virtual void setup(
        const int pSamplerate, const int pMaxBuffer, const int pInputs = 1,
        const int pOutputs = 1, const int pChannels = 2
    ) {
      mMaxBlockSize = pMaxBuffer;
      mInputCount = pInputs;
      mOutputCount = pOutputs;
      mChannelCount = pChannels;

      // Setup the sockets for the node connections
      for (int i = 0; i < mInputCount; i++) {
        mSocketsIn[i].mParentNode = this;
        mSocketsIn[i].mIndex = i;
        mSocketsIn[i].mIsInput = true;
      }

      for (int i = 0; i < mOutputCount; i++) {
        mSocketsOut[i].mParentNode = this;
        mSocketsOut[i].mIndex = i;
        mSocketsOut[i].mIsInput = false;
      }

      // This will create all the needed buffers
      OnReset(pSamplerate, pChannels, true);
    }

    /**
     * Create all the needed buffers for the dsp
     * Called from on reset when the channel count changes
     */
    virtual void createBuffers() {
      for (int i = 0; i < mOutputCount; i++) {
        mSocketsOut[i].mBuffer = new sample* [mChannelCount];
        for (int c = 0; c < mChannelCount; c++) {
          mSocketsOut[i].mBuffer[c] = new sample[mMaxBlockSize];
        }
      }
    }

    /**
     * Deletes all the allocated audio buffers
     */
    virtual void deleteBuffers() {
      for (int i = 0; i < mOutputCount; i++) {
        if (mSocketsOut[i].mBuffer != EMPTY_BUFFER) {
          for (int c = 0; c < mChannelCount; c++) {
            delete[] mSocketsOut[i].mBuffer[c];
          }
          delete[] mSocketsOut[i].mBuffer;
          mSocketsOut[i].mBuffer = EMPTY_BUFFER;
        }
      }
    }

    /**
     * Usually clean up should happen deleteBuffers() or cleanUp()
     */
    virtual ~Node() = default;

    /**
     * Called right before the node is destroyed
     * will get rid of the automations, audio buffers and sockets
     */
    virtual void cleanUp() {
      deleteBuffers();
      for (int i = 0; i < mParameterCount; i++) {
        detachAutomation(&mParameters[i]); // Make sure no automation is attached
      }
      // will disconnect all sockets
      delete mOverSampler;
    }

    /**
     * Will fill all the output buffers with silence and set the processed flag to true
     */
    void outputSilence() const {
      for (int o = 0; o < mOutputCount; o++) {
        for (int c = 0; c < mChannelCount; c++) {
          for (int i = 0; i < mMaxBlockSize; i++) {
            mSocketsOut[o].mBuffer[c][i] = 0;
          }
        }
      }
    }

    /**
     * Will return true if the node is bypassed and also do the bypassing of buffers
     */
    bool byPass() const {
      mParameters[mByPassedIndex].update();
      if (mByPassed < 0.5) { return false; }
      sample** in = mSocketsIn[0].mBuffer;
      for (int o = 0; o < mOutputCount; o++) {
        for (int c = 0; c < mChannelCount; c++) {
          for (int i = 0; i < mMaxBlockSize; i++) {
            mSocketsOut[o].mBuffer[c][i] = in[c][i];
          }
        }
      }
      return true;
    }

    /**
     * Main Processing, only takes a blocksize since the node knows its inputs
     */
    virtual void ProcessBlock(int nFrames) = 0;


    /**
     * Signals a new audio block is about to processed
     */
    virtual void BlockStart() { }

    /**
     * Called if the daw changed the channel count
     */
    virtual void OnChannelsChanged(const int pChannels) {
      deleteBuffers();
      mChannelCount = pChannels;
      createBuffers();
    }

    /**
     * Called on each block to handle a changing oversampling factor
     */
    void updateOversampling() {
      if (mOverSampler != nullptr) {
        mParameters[mOverSamplingIndex].update();
        int fac = std::floor(mOverSamplingFactor);
        if (fac != mOverSampler->mFactor && mSampleRate > 0) {
          float prevFac = mOverSampler->mFactor;
          mOverSampler->mFactor = fac;
          OnSamplerateChanged(mSampleRate / prevFac);
        }
      }
    }

    /**
     * Called once when the node is constructed to allow oversampling later on
     */
    virtual void enableOversampling() {
      if (mOverSampler == nullptr) {
        mOverSamplingFactor = 1.0;
        mOverSampler = new Oversampler();
        mOverSamplingIndex = addParameter("OverSampling", &mOverSamplingFactor, 1.0, 1, 4, 1);
      }
    }

    /**
     * React to a change in sample rate
     * Call the base implementation first and then use the value
     * stored in mSampleRate to get the oversampling right
     */
    virtual void OnSamplerateChanged(const int pSampleRate) {
      if (mOverSampler != nullptr) {
        mSampleRate = pSampleRate * mOverSampler->mFactor;
      }
      else {
        mSampleRate = pSampleRate;
      }
    }

    /**
     * Called on DAW transport e.g. to clear dsp buffers and cut reverbs
     */
    virtual void OnTransport() { }

    /**
     * Called from the graph to either signal a change in samplerate/channel count or transport
     */
    virtual void OnReset(const int pSampleRate, const int pChannels, const bool force = false) {
      if (pSampleRate != mSampleRate || force) {
        OnSamplerateChanged(pSampleRate);
      }
      if (pChannels != mChannelCount || force) {
        OnChannelsChanged(pChannels);
      }
      OnConnectionsChanged();
    }

    /**
     * Means the in or out connections changed, get the pointer to the right buffer again
     * CALL BASE IMPLEMENTATION
     */
    virtual void OnConnectionsChanged() {
      outputSilence();
      for (int i = 0; i < mInputCount; i++) {
        if (mSocketsIn[i].mConnected) {
          mSocketsIn[i].mBuffer = mSocketsIn[i].mConnectedTo[0]->mBuffer;
        }
        else {
          mSocketsIn[i].mBuffer = EMPTY_BUFFER;
        }
      }

      memset(mDependencies, 0, GUITARD_MAX_NODE_SOCKETS + GUITARD_MAX_NODE_PARAMETERS * sizeof(Node*));
      mDependencyCount = 0;
      for (int i = 0; i < mInputCount; i++) {
        if (mSocketsIn[i].mConnected) {
          Node* parent = mSocketsIn[i].mConnectedTo[0]->mParentNode;
          if (parent->mInfo->name != "InputNode" && parent->mInfo->name != "FeedbackNode") {
            mDependencies[mDependencyCount] = mSocketsIn[i].mConnectedTo[0]->mParentNode;
            mDependencyCount++;
          }
        }
      }
      for (int i = 0; i < mParameterCount; i++) {
        if (mParameters[i].automationDependency != nullptr) {
          mDependencies[mDependencyCount] = mParameters[i].automationDependency;
          mDependencyCount++;
        }
      }
    }

    /**
     * This is called from the CableLayer
     * @param n The Node which is the automation source since it needs to be added to the list of nodes it depends on
     * @param index Is the index of the ParameterCoupling to attach the automation to
     */
    virtual void attachAutomation(Node* n, const int index) {
      if (index >= mParameterCount) { return; }
      ParameterCoupling* p = &mParameters[index];
      n->addAutomationTarget(p);
      OnConnectionsChanged();
    }

    /**
     * Removes the node from the dependency list and removes all the automation targets
     * from the automation node
     * @param p The ParameterCouple to remove the automation from
     */
    virtual void detachAutomation(ParameterCoupling* p) {
      if (p != nullptr) {// TODO check whether a p is actually part of mParameters
        if (p->automationDependency != nullptr) {
          p->automationDependency->removeAutomationTarget(p);
        }
      }
      OnConnectionsChanged();
    }

    /**
     * This is for nodes that can provide automation for other nodes
     * They will keep track of all the ParameterCouplings and update them accordingly
     * These should only be called from the attach/detachAutomation functions
     * @param c The ParameterCouple to control
     */
    virtual void addAutomationTarget(ParameterCoupling* c) { }

    /**
     * Also for nodes which provide automation, see above
     * @param c The ParameterCouple to control
     */
    virtual void removeAutomationTarget(ParameterCoupling* c) { }


    /**
     * Called once to add a bypass parameter
     */
    void addByPassParam() {
      if (mByPassedIndex != -1) {
        assert(false);
        return;
      }
      mByPassedIndex = addParameter("Bypass", &mByPassed, 0.0, 0.0, 1.0, 1);
    }

    /**
     * Generic function to call when the node can switch between mono/stereo
     * @param p a Coupling from outside to use. If none is provided a new one will be used
     */
    void addStereoParam(ParameterCoupling* p = nullptr) {
      if (p == nullptr) {
        addParameter("Stereo", &mStereo, 0.0, 0.0, 1.0, 1);
      }
      else {
        addParameter(*p);
      }

    }

    /**
     * Adds a Parameter
     * @name Parameter name used for serialization and display
     * @prop A pointer to the float/double value controlled by it
     * @propo def The default value
     * @prop min Minimum value
     * @prop max Maximum value
     * @prop Stepsize for the gui precision
     */
    int addParameter(const char* name, sample* prop, sample def, sample min, sample max, sample stepSize, Coord2D pos = {0, 0}) {
      int index = addParameter(ParameterCoupling(name, prop, def, min, max, stepSize));
      if (index >= 0) {
        mParameters[index].pos = pos;
      }
      return index;
    }

    /**
     * Adds a parametercoupling
     */
    int addParameter(const ParameterCoupling p) {
      if (mParameterCount >= GUITARD_MAX_NODE_PARAMETERS) { return -1; }
      mParameters[mParameterCount] = p;
      mParameterCount++;
      return mParameterCount - 1;
    }

    void addMeter(const char* name, sample* prop, sample min, sample max) {
      *prop = 0; // They never get initialized in the Faust code
      mMeters[mMeterCount] = MeterCoupling{ prop, name, min, max };
      mMeterCount++;
    }

    /**
     * Copies over the state of the given node if it's from the same type
     * @param n The node to copy the state from
     */
    void copyState(Node* n) {
      if (mInfo->name != n->mInfo->name) { // Check the type
        WDBGMSG("Trying to copy a state from a different node type!\n");
        assert(false);
        return;
      }

      for (int i = 0; i < mParameterCount; i++) { // Synchronize all the parameters
        mParameters[i].setValue(n->mParameters[i].getValue());
      }

      // Carry over the additional data
      nlohmann::json temp;
      n->serializeAdditional(temp);
      deserializeAdditional(temp);
    }

    /**
     * Allows attaching additional data at the end of the serialization
     */
    virtual void serializeAdditional(nlohmann::json& serialized) { }

    /**
     * Allows loading additional settings after deserialization
     */
    virtual void deserializeAdditional(nlohmann::json& serialized) { }

    /**
     * Function to retrieve the license/copyright info about the node
     */
    virtual String getLicense() {
      return "No copyright info provided.";
    }
  };
}