#pragma once

#include "IPlugConstants.h"
#include "IGraphics.h"
#include "src/node/NodeUi.h"
#include "src/node/NodeSocket.h"
#include "NodeShared.h"

/**
 * Virtual class which all nodes will derive from
 */
class Node {
protected:
  bool mUiReady = false;
public:
  NodeShared shared;

  // Flag to skip automation if there's none
  bool mIsAutomated = false;
  // The dsp will write the result here and it will be exposed to other nodes over the NodeSocket
  sample*** mBuffersOut = nullptr;
  // The UI element representing this node instance visually
  NodeUi* mUi = nullptr;
  bool mIsProcessed = false;
  
  int mSampleRate = 0;
  int mChannelCount = 0;
  int mLastBlockSize = 0;

  sample mByPassed = 0;
  sample mStereo = 1;

  /**
   * This is basically a delayed constructor with the only disadvantage: derived methods have to have the same parameters
   * The derived class will call this with the desired parameters, except for the samplerate
   */
  virtual void setup(MessageBus::Bus* pBus, const int pSamplerate,
                     const int pMaxBuffer, const int pChannles = 2,
                     const int pInputs = 1, const int pOutputs = 1)
  {
    shared.bus = pBus;
    shared.node = this;
    mSampleRate = 0;
    mChannelCount = 0;
    shared.maxBlockSize = pMaxBuffer;
    shared.inputCount = pInputs;
    shared.outputCount = pOutputs;
    mIsProcessed = false;
    mUiReady = false;
    // Setup the sockets for the node connections
    for (int i = 0; i < shared.inputCount; i++) {
      shared.socketsIn[i] = new NodeSocketIn(this, i);
    }

    for (int i = 0; i < shared.outputCount; i++) {
      shared.socketsOut[i] = new NodeSocketOut(this, i);
    }

    // This will create all the needed buffers
    OnReset(pSamplerate, pChannles);



    positionSockets();
  }

  /**
   * Create all the needed buffers for the dsp
   * Called from on reset when the channelcount changes
   */
  virtual void createBuffers() {
    if (mBuffersOut != nullptr) {
      WDBGMSG("Trying to create a new dsp buffer without cleanung up the old one");
      assert(false);
    }
    mBuffersOut = new sample **[shared.outputCount];
    for (int i = 0; i < shared.outputCount; i++) {
      mBuffersOut[i] = new sample * [mChannelCount];
      for (int c = 0; c < mChannelCount; c++) {
        mBuffersOut[i][c] = new sample[shared.maxBlockSize];
      }
      shared.socketsOut[i]->mParentBuffer = mBuffersOut[i]; // Need to inform the outputs about the buffer
    }
  }

  /**
   * Deletes all the allocated buffers
   */
  virtual void deleteBuffers() {
    if (mBuffersOut != nullptr) {
      for (int i = 0; i < shared.outputCount; i++) {
        for (int c = 0; c < mChannelCount; c++) {
          delete mBuffersOut[i][c];
        }
        delete mBuffersOut[i];
        shared.socketsOut[i]->mParentBuffer = nullptr;
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
   */
  virtual void cleanUp() {
    deleteBuffers();

    if (mUiReady || mUi != nullptr) {
      WDBGMSG("Warning, UI of node was not cleaned up!\n");
    }

    for (int i = 0; i < shared.parameterCount; i++) {
      detachAutomation(&shared.parameters[i]);
    }

    for (int i = 0; i < shared.meterCount; i++) {
      delete shared.meters[i];
    }

    for (int i = 0; i < shared.inputCount; i++) {
      shared.socketsIn[i]->disconnectAll();
      delete shared.socketsIn[i];
    }

    for (int i = 0; i < shared.outputCount; i++) {
      shared.socketsOut[i]->disconnectAll();
      delete shared.socketsOut[i];
    }
  }

  /**
   * Will fill all the output buffers with silence and set the processed flag to true
   */
  void outputSilence() {
    for (int o = 0; o < shared.outputCount; o++) {
      for (int c = 0; c < mChannelCount; c++) {
        for (int i = 0; i < shared.maxBlockSize; i++) {
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
    // The first param will always be bypass
    shared.parameters[0].update();
    if (mByPassed < 0.5) { return false; }
    sample** in = shared.socketsIn[0]->mConnectedTo[0]->mParentBuffer;
    for (int o = 0; o < shared.outputCount; o++) {
      for (int c = 0; c < mChannelCount; c++) {
        for (int i = 0; i < shared.maxBlockSize; i++) {
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
    for (int i = 0; i < shared.inputCount; i++) {
      if (shared.socketsIn[i]->mConnectedTo[0] == nullptr) {
        outputSilence();
        return false;
      }
    }

    /**
     * Check for inputs which are connected to unprocessed nodes
     */
    for (int i = 0; i < shared.inputCount; i++) {
      if (!shared.socketsIn[i]->mConnectedTo[0]->mParentNode->mIsProcessed) {
        // A node isn't ready so return false
        return false;
      }
    }

    /**
     * Check for automation
     */
    if (mIsAutomated) {
      for (int i = 0; i < shared.parameterCount; i++) {
        Node* n = shared.parameters[i].automationDependency;
        if (n != nullptr && !n->mIsProcessed) {
          return false;
        }
      }
    }

    return true;
  }

  void checkIsAutomated() {
    for (int i = 0; i < shared.parameterCount; i++) {
      if (shared.parameters[i].automationDependency != nullptr) {
        mIsAutomated = true;
        return;
      }
    }
    mIsAutomated = false;
  }

  /** Main Processing, only takes a blocksize since it knows its inputs */
  virtual void ProcessBlock(int nFrames) = 0;



  /**                  Signals from outside                  */

  /** Signals a new audio block is about to processed */
  virtual void BlockStart() {
    mIsProcessed = false;
  }

  /** Called if the daw changed the channelcount*/
  virtual void OnChannelsChanged(const int pChannels) {
    deleteBuffers();
    mChannelCount = pChannels;
    createBuffers();
  }

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
   */
  virtual void connectInput(NodeSocket* out, const int inputNumber = 0) {
    NodeSocket* inSocket = shared.socketsIn[inputNumber];
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
   * In order for automations to keep in sync inside a audio block
   * the automation node responsible for the automation has to be added to a
   * dependency list.
   * The requested ParameterCoupling is returned to be used in the automation node
   */
  virtual void attachAutomation(Node* n, const int index) {
    ParameterCoupling* p = &shared.parameters[index];
    if (p != nullptr) {
      n->addAutomationTarget(p);
    }
    checkIsAutomated();
  }

  /**
   * Removes the node from the dependency list and removes all the automation targets
   * from the automation node
   */
  virtual void detachAutomation(ParameterCoupling* p) {
    // ParameterCoupling* p = mParameters.Get(index);
    if (p != nullptr) {
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
   */
  virtual void addAutomationTarget(ParameterCoupling* c) { }

  /**
   * Also for nodes which provide automation, see above
   */
  virtual void removeAutomationTarget(ParameterCoupling* c) { }


  /**
   * ALWAYS NEEDS TO BE THE FIRST PARAM IF USED
   * Generic function to call when the node can be bypassed
   */
  void addByPassParam() {
    shared.parameters[shared.parameterCount] = ParameterCoupling(
      "Bypass", &mByPassed, 0.0, 0.0, 1.0, 1
    );
    shared.parameterCount++;
  }

  /** Generic function to call when the node can switch between mono/stereo */
  void addStereoParam(ParameterCoupling* p = nullptr) {
    if (p == nullptr) {
      shared.parameters[shared.parameterCount] = ParameterCoupling(
        "Stereo", &mStereo, 0.0, 0.0, 1.0, 1
      );
    }
    else {
      shared.parameters[shared.parameterCount] = *p;
    }
    
    shared.parameterCount++;
  }

  /** Copies over the state of the given node if it's from the same type */
  void copyState(Node* n) {
    if (shared.type != n->shared.type) {
      WDBGMSG("Trying to copy a state from a different node type!\n");
      assert(false);
      return;
    }
    for (int i = 0; i < shared.parameterCount; i++) {
      shared.parameters[i].setValue(n->shared.parameters[i].getValue());
    }
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


  /**
   * Generic setup of the parameters to get something on the screen
   */
  virtual void setupUi(IGraphics* pGrahics) {
    shared.graphics = pGrahics;
    mUi = new NodeUi(&shared);
    mUi->setColor(IColor(255, 100, 100, 100));
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    mUiReady = true;
  }

  virtual void positionSockets() {
    for (int i = 0; i < shared.inputCount; i++) {
      NodeSocket* s = shared.socketsIn[i];
      s->mX = shared.X - shared.width * 0.5;
      s->mY = i * 50.f + shared.Y;
    }

    for (int i = 0; i < shared.outputCount; i++) {
      NodeSocket* s = shared.socketsOut[i];
      s->mX = shared.width * 0.5 + shared.X - 30;
      s->mY = i * 50.f + shared.Y;
    }
  }

  /**
   * Cleans up the IControls for all the parameters
   */
  virtual void cleanupUi(IGraphics* pGraphics) {
    if (mUi != nullptr) {
      mUi->cleanUp();
      pGraphics->RemoveControl(mUi, true);
      mUi = nullptr;
    }
    mUiReady = false;
  }

  void moveAlong(const float x) {
    if (shared.type == "FeedbackNode") { return; }
    if (mUi != nullptr) {
      mUi->translate(x, 0);
    }
    for (int i = 0; i < shared.outputCount; i++) {
      NodeSocket* socket = shared.socketsOut[i];
      for (int s = 0; s < MAX_SOCKET_CONNECTIONS; s++) {
        if (socket->mConnectedTo[s] != nullptr) {
          socket->mConnectedTo[s]->mParentNode->moveAlong(x);
        }
      }
    }
  }

  /**
   * Function to retrieve the license/copyright info about the node
   */
  virtual std::string getLicense() {
    return "Not set";
  }
};
