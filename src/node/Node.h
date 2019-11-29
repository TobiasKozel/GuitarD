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
  std::string mType;

  // Flag to skip automation if there's none
  bool mIsAutomated = false;
  // The dsp will write the result here and it will be exposed to other nodes over the NodeSocket
  sample*** mBuffersOut = nullptr;
  // The UI element representing this node instance visually
  NodeUi* mUi = nullptr;
  bool mIsProcessed = false;
  
  int mSampleRate = 0;
  int mChannelCount = 0;
  int mMaxBuffer = 0;
  int mLastBlockSize = 0;

  double mByPassed = 0;
  double mStereo = 1;

  /**
   * This is basically a delayed constructor with the only disadvantage: derived methods have to have the same parameters
   * The derived class will call this with the desired parameters, except for the samplerate
   */
  virtual void setup(MessageBus::Bus* pBus, const int pSamplerate = 48000,
                     const int pMaxBuffer = MAX_BUFFER, const int pChannles = 2,
                     const int pInputs = 1, const int pOutputs = 1)
  {
    shared.bus = pBus;
    shared.node = this;
    mSampleRate = 0;
    mChannelCount = 0;
    mMaxBuffer = pMaxBuffer;
    shared.inputCount = pInputs;
    shared.outputCount = pOutputs;
    mIsProcessed = false;
    mUiReady = false;
    OnReset(pSamplerate, pChannles);

    // Setup the sockets for the node connections
    for (int i = 0; i < shared.inputCount; i++) {
      shared.socketsIn[i] = new NodeSocket(shared.bus, i, this);
    }

    for (int i = 0; i < shared.outputCount; i++) {
      shared.socketsOut[i] = new NodeSocket(shared.bus, i, this, mBuffersOut[i]);
    }

    autoAllignSockets();
  }

  /**
   * Create all the needed buffers for the dsp
   * Called from on reset when the channelcount changes
   */
  virtual void createBuffers() {
    if (mBuffersOut != nullptr) {
      WDBGMSG("Trying to create a new dsp buffer without cleanung up the old one");
      assert(true);
    }
    mBuffersOut = new sample **[shared.outputCount];
    for (int i = 0; i < shared.outputCount; i++) {
      mBuffersOut[i] = new sample * [mChannelCount];
      for (int c = 0; c < mChannelCount; c++) {
        mBuffersOut[i][c] = new sample[mMaxBuffer];
      }
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
      detachAutomation(shared.parameters[i]);
      delete shared.parameters[i];
    }

    for (int i = 0; i < shared.meterCount; i++) {
      delete shared.meters[i];
    }

    for (int i = 0; i < shared.inputCount; i++) {
      delete shared.socketsIn[i];
    }

    for (int i = 0; i < shared.outputCount; i++) {
      delete shared.socketsOut[i];
    }
  }

  /**
   * Will fill all the output buffers with silence and set the processed flag to true
   */
  void outputSilence() {
    for (int o = 0; o < shared.outputCount; o++) {
      for (int c = 0; c < mChannelCount; c++) {
        for (int i = 0; i < mMaxBuffer; i++) {
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
    shared.parameters[0]->update();
    if (mByPassed < 0.5) { return false; }
    sample** in = shared.socketsIn[0]->mConnectedTo->mParentBuffer;
    for (int o = 0; o < shared.outputCount; o++) {
      for (int c = 0; c < mChannelCount; c++) {
        for (int i = 0; i < mMaxBuffer; i++) {
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
      if (shared.socketsIn[i]->mConnectedTo == nullptr) {
        outputSilence();
        return false;
      }
    }

    /**
     * Check for inputs which are connected to unprocessed nodes
     */
    for (int i = 0; i < shared.inputCount; i++) {
      if (!shared.socketsIn[i]->mConnectedTo->mParentNode->mIsProcessed) {
        // A node isn't ready so return false
        return false;
      }
    }

    /**
     * Check for automation
     */
    if (mIsAutomated) {
      for (int i = 0; i < shared.parameterCount; i++) {
        Node* n = shared.parameters[i]->automationDependency;
        if (n != nullptr && !n->mIsProcessed) {
          return false;
        }
      }
    }

    return true;
  }

  void checkIsAutomated() {
    for (int i = 0; i < shared.parameterCount; i++) {
      if (shared.parameters[i]->automationDependency != nullptr) {
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
   * TODOG doesn't seem to work
   */
  virtual void OnTransport() { }

  /**
   * Called from the graph to either signal a change in samplerate/channel count or transport
   */
  virtual void OnReset(const int pSampleRate, const int pChannels) {
    bool isTransport = true;
    if (pSampleRate != mSampleRate) {
      OnSamplerateChanged(pSampleRate);
      isTransport = false;
    }
    if (pChannels != mChannelCount) {
      OnChannelsChanged(pChannels);
      isTransport = false;
    }
    if (isTransport) {
      OnTransport();
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
    ParameterCoupling* p = shared.parameters[index];
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
    shared.parameters[shared.parameterCount] = new ParameterCoupling(
      "Bypass", &mByPassed, 0.0, 0.0, 1.0, 1
    );
    shared.parameterCount++;
  }

  /** Generic function to call when the node can switch between mono/stereo */
  void addStereoParam(ParameterCoupling* p = nullptr) {
    if (p == nullptr) {
      p = new ParameterCoupling(
        "Stereo", &mStereo, 1.0, 0.0, 1.0, 1
      );
    }
    shared.parameters[shared.parameterCount] = p;
    shared.parameterCount++;
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

  virtual void autoAllignSockets() {
    for (int i = 0; i < shared.inputCount; i++) {
      NodeSocket* s = shared.socketsIn[i];
      if (s->mX == s->mY && s->mX == 0) {
        s->mX = 0;
        s->mY = i * 50.f + shared.height * 0.5f;
      }
    }

    for (int i = 0; i < shared.outputCount; i++) {
      NodeSocket* s = shared.socketsOut[i];
      if (s->mX == 0) {
        s->mX = shared.width - 30;
      }
      if (s->mY == 0) {
        s->mY = i * 50.f + shared.height * 0.5f;
      }
    }
  }

  /**
   * Cleans up the IControls for all the parameters
   */
  virtual void cleanupUi(IGraphics* pGrahics) {
    /**
     * The param value gets only synced to the dsp value when the node is processed
     * If the node is not connected this won't happen, so always do the update when the
     * Gui window is closed just in case
     */
    for (int i = 0; i < shared.parameterCount; i++) {
      shared.parameters[i]->update();
    }
    if (mUi != nullptr) {
      mUi->cleanUp();
      pGrahics->RemoveControl(mUi, true);
      mUi = nullptr;
    }
    mUiReady = false;
  }

  /** Called if the ui is resized TODOG move over to NodeUI */
  virtual void layoutChanged() { }
};

