#pragma once

#include "../misc/NodeList.h" // We'll need the list to register the nodes to it
#include "../node/NodeSocket.h"
#include "../parameter/ParameterCoupling.h"
#include "../parameter/MeterCoupling.h"
#include "../types/gstructs.h"
#include "../types/types.h"
#include "Oversampler.h"

namespace guitard {
  /**
   * Virtual class which all nodes will derive from
   * It's the DSP part of the node
   */
  class Node {
    iplug::OverSampler<sample>* mOverSampler = nullptr;
  public: // Everything is public since it most of it needs to be accessible from the graph and the NodeUi
    bool mIsAutomated = false; // Flag to skip automation if there's none
    // The dsp will write the result here and it will be exposed to other nodes over the NodeSocket
    sample*** mBuffersOut = nullptr;
    bool mIsProcessed = false;

    int mOverSamplingFactor = 1;
    int mSampleRate = 0;
    int mLastBlockSize = 0;
    // This size will be used to allocate the dsp buffer, the actual samples per block can be lowe
    int mMaxBlockSize = 0; 
    int mChannelCount = 0;

    sample mByPassed = 0;
    sample mStereo = 1;

    int mParameterCount = 0;
    ParameterCoupling mParameters[MAX_NODE_PARAMETERS];
    int mMeterCount = 0;
    MeterCoupling mMeters[MAX_NODE_METERS];

    int mInputCount = 0;
    NodeSocket* mSocketsIn[MAX_NODE_SOCKETS] = { nullptr };
    int mOutputCount = 0;
    NodeSocket* mSocketsOut[MAX_NODE_SOCKETS] = { nullptr };

    Coord2D mPos = { 0, 0 }; // Position on the canvas in pixels
    Coord2D mDimensions = { 250, 200 }; // Size in Pixels
    NodeList::NodeInfo* mInfo;

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
      mIsProcessed = false;
      mChannelCount = pChannels;

      // Setup the sockets for the node connections
      for (int i = 0; i < mInputCount; i++) {
        mSocketsIn[i] = new NodeSocketIn(this, i); 
      }

      for (int i = 0; i < mOutputCount; i++) {
        mSocketsOut[i] = new NodeSocketOut(this, i);
      }

      // This will create all the needed buffers
      OnReset(pSamplerate, pChannels, true);
    }

    /**
     * Create all the needed buffers for the dsp
     * Called from on reset when the channel count changes
     */
    virtual void createBuffers() {
      if (mBuffersOut != nullptr) {
        WDBGMSG("Trying to create a new dsp buffer without cleanung up the old one");
        assert(false);
      }
      mBuffersOut = new sample** [mOutputCount];
      for (int i = 0; i < mOutputCount; i++) {
        mBuffersOut[i] = new sample* [mChannelCount];
        for (int c = 0; c < mChannelCount; c++) {
          mBuffersOut[i][c] = new sample[mMaxBlockSize];
        }
        mSocketsOut[i]->mParentBuffer = mBuffersOut[i]; // Need to inform the outputs about the buffer
      }
    }

    /**
     * Deletes all the allocated audio buffers
     */
    virtual void deleteBuffers() {
      if (mBuffersOut != nullptr) {
        for (int i = 0; i < mOutputCount; i++) {
          for (int c = 0; c < mChannelCount; c++) {
            delete mBuffersOut[i][c];
          }
          delete mBuffersOut[i];
          mSocketsOut[i]->mParentBuffer = nullptr;
        }
        delete mBuffersOut;
        mBuffersOut = nullptr;
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

      for (int i = 0; i < mInputCount; i++) {
        mSocketsIn[i]->disconnectAll();
        delete mSocketsIn[i];
      }

      for (int i = 0; i < mOutputCount; i++) {
        mSocketsOut[i]->disconnectAll();
        delete mSocketsOut[i];
      }
    }

    /**
     * Will fill all the output buffers with silence and set the processed flag to true
     */
    void outputSilence() {
      for (int o = 0; o < mOutputCount; o++) {
        for (int c = 0; c < mChannelCount; c++) {
          for (int i = 0; i < mMaxBlockSize; i++) {
            mBuffersOut[o][c][i] = 0;
          }
        }
      }
      mIsProcessed = true;
    }

    /**
     * Will return true if the node is bypassed and also do the bypassing of buffers
     */
    bool byPass() {
      mParameters[0].update(); // The first param will always be bypass
      if (mByPassed < 0.5) { return false; }
      sample** in = mSocketsIn[0]->mConnectedTo[0]->mParentBuffer;
      for (int o = 0; o < mOutputCount; o++) {
        for (int c = 0; c < mChannelCount; c++) {
          for (int i = 0; i < mMaxBlockSize; i++) {
            mBuffersOut[o][c][i] = in[c][i];
          }
        }
      }
      mIsProcessed = true;
      return true;
    }

    /**
     * Check whether the node is able to process a block
     * Will also output silence if not connected
     */
    virtual bool inputsReady() {
      /**
       * Check for disconnected inputs
       * If one input isn't connected, skip the processing and output silence
       * If that's not desired, this function has to be overriden
       */
      for (int i = 0; i < mInputCount; i++) {
        if (mSocketsIn[i]->mConnectedTo[0] == nullptr) {
          outputSilence();
          return false;
        }
      }

      /**
       * Check for inputs which are connected to unprocessed nodes
       */
      for (int i = 0; i < mInputCount; i++) {
        if (!mSocketsIn[i]->mConnectedTo[0]->mParentNode->mIsProcessed) {
          return false; // A node isn't ready so return false
        }
      }

      /**
       * Check for automation since it needs to be ready as well
       */
      if (mIsAutomated) {
        for (int i = 0; i < mParameterCount; i++) {
          Node* n = mParameters[i].automationDependency;
          if (n != nullptr && !n->mIsProcessed) {
            return false;
          }
        }
      }

      return true;
    }

    /**
     * Updates the internal mIsAutomated state
     */
    void checkIsAutomated() {
      for (int i = 0; i < mParameterCount; i++) {
        if (mParameters[i].automationDependency != nullptr) {
          mIsAutomated = true;
          return;
        }
      }
      mIsAutomated = false;
    }

    /**
     * Main Processing, only takes a blocksize since the node knows its inputs
     */
    virtual void ProcessBlock(int nFrames) = 0;


    /**
     * Signals a new audio block is about to processed
     */
    virtual void BlockStart() {
      mIsProcessed = false;
    }

    /**
     * Called if the daw changed the channel count
     */
    virtual void OnChannelsChanged(const int pChannels) {
      deleteBuffers();
      mChannelCount = pChannels;
      createBuffers();
    }

    void setOverSampling(int fac) {
      if (mOverSampler != nullptr && mOverSamplingFactor != fac) {
        mOverSampler->SetOverSampling(iplug::OverSampler<sample>::RateToFactor(fac));
        OnSamplerateChanged((mSampleRate / mOverSamplingFactor) * fac);
        mOverSamplingFactor = fac;
      }
    }

    /**
     * React to a change in sample rate
     * since it won't affect the buffers and the dsp usually needs
     * to change some internal values, we can't deal with it here
     */
    virtual void OnSamplerateChanged(const int pSampleRate) {
      mSampleRate = pSampleRate;
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
    }

    /**
     * Connects a given socket to a input at a given index of this node
     * @param out The output socket to connect
     * @param inputNumber The index of the input socket to connect the out socket to
     */
    virtual void connectInput(NodeSocket* out, const int inputNumber = 0) {
      NodeSocket* inSocket = mSocketsIn[inputNumber];
      if (inSocket != nullptr) {
        if (out == nullptr) {
          inSocket->disconnect();
        }
        else {
          inSocket->connect(out);
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
      checkIsAutomated();
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
      checkIsAutomated();
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
     * ALWAYS NEEDS TO BE THE FIRST PARAM IF USED
     * Generic function to call when the node can be bypassed
     */
    void addByPassParam() {
      if (mParameterCount != 0) {
        assert(false);
        return ;
      }
      mParameters[mParameterCount] = ParameterCoupling(
        "Bypass", &mByPassed, 0.0, 0.0, 1.0, 1
      );
      mParameterCount++;
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
    void addParameter(const char* name, sample* prop, sample def, sample min, sample max, sample stepSize) {
      addParameter(ParameterCoupling(name, prop, def, min, max, stepSize));
    }

    /**
     * Adds a parametercoupling
     */
    void addParameter(const ParameterCoupling p) {
      if (mParameterCount >= MAX_NODE_PARAMETERS) { return; }
      mParameters[mParameterCount] = p;
      mParameterCount++;
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


    /**                 UI STUFF                */
    // TODO this has to go

#ifndef GUITARD_HEADLESS
    //void moveAlong(const float x) {
    //  if (shared.info->name == "FeedbackNode") { return; }
    //  if (mUi != nullptr) {
    //    mUi->translate(x, 0);
    //  }
    //  for (int i = 0; i < shared.outputCount; i++) {
    //    NodeSocket* socket = shared.socketsOut[i];
    //    for (int s = 0; s < MAX_SOCKET_CONNECTIONS; s++) {
    //      if (socket->mConnectedTo[s] != nullptr) {
    //        socket->mConnectedTo[s]->mParentNode->moveAlong(x);
    //      }
    //    }
    //  }
    //}
#endif

    /**
     * Function to retrieve the license/copyright info about the node
     */
    virtual String getLicense() {
      return "No copyright info provided.";
    }
  };
}