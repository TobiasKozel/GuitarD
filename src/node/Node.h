#pragma once

#include "IPlugConstants.h"
#include "IGraphics.h"
#include "src/node/NodeUi.h"
#include "src/node/NodeSocket.h"
#include "src/parameter/ParameterCoupling.h"
#include "src/misc/MessageBus.h"

/**
 * Virtual class which all nodes will derive from
 */
class Node {
protected:
  MessageBus::Bus* mBus;
  bool mUiReady;
public:
  std::string mType;

  WDL_PtrList<ParameterCoupling> mParameters;
  // The dsp will get the data from the buffer inside the socket
  int mInputCount = 0;
  WDL_PtrList<NodeSocket> mSocketsIn;
  int mOutputCount = 0;
  WDL_PtrList<NodeSocket> mSocketsOut;
  // Flag to skip automation if there's none
  bool mIsAutomated = false;
  // The dsp will write the result here and it will be exposed to other nodes over the NodeSocket
  iplug::sample*** mBuffersOut = nullptr;
  // The UI element representing this node instance visually
  NodeUi* mUi = nullptr;
  bool mIsProcessed = false;
  
  float mX = 0;
  float mY = 0;
  float rotation = 0;

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
  virtual void setup(MessageBus::Bus* pBus, int p_samplerate = 48000, int p_maxBuffer = MAX_BUFFER, int p_channles = 2, int p_inputs = 1, int p_outputs = 1) {
    mBus = pBus;
    mSampleRate = 0;
    mChannelCount = 0;
    mMaxBuffer = p_maxBuffer;
    mInputCount = p_inputs;
    mOutputCount = p_outputs;
    mIsProcessed = false;
    mUiReady = false;
    OnReset(p_samplerate, p_channles);

    // Setup the sockets for the node connections
    for (int i = 0; i < mInputCount; i++) {
      NodeSocket* in = new NodeSocket(mBus, i, this);
      mSocketsIn.Add(in);
    }

    for (int i = 0; i < mOutputCount; i++) {
      NodeSocket* out = new NodeSocket(mBus, i, this, mBuffersOut[i]);
      mSocketsOut.Add(out);
    }
  }

  /**
   * Create all the needed buffers for the dsp
   */
  virtual void createBuffers() {
    if (mBuffersOut != nullptr) {
      WDBGMSG("Trying to create a new dsp buffer without cleanung up the old one");
      assert(true);
    }
    mBuffersOut = new iplug::sample **[mOutputCount];
    for (int i = 0; i < mOutputCount; i++) {
      mBuffersOut[i] = new iplug::sample * [mChannelCount];
      for (int c = 0; c < mChannelCount; c++) {
        mBuffersOut[i][c] = new iplug::sample[mMaxBuffer];
      }
    }
  }

  /**
   * Deletes all the allocated buffers
   */
  virtual void deleteBuffers() {
    if (mBuffersOut != nullptr) {
      for (int i = 0; i < mOutputCount; i++) {
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
   * Should do all the required cleanup
   */
  virtual ~Node() {
    deleteBuffers();

    if (mUiReady || mUi != nullptr) {
      WDBGMSG("Warning, UI of node was not cleaned up!\n");
    }

    for (int i = 0; i < mParameters.GetSize(); i++) {
      detachAutomation(mParameters.Get(i));
    }
    mParameters.Empty(true);
    mSocketsIn.Empty(true);
    mSocketsOut.Empty(true);
  }

  /**
   * Will fill all the output buffers with silence
   */
  void outputSilence() {
    for (int o = 0; o < mOutputCount; o++) {
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
    mParameters.Get(0)->update();
    if (mByPassed < 0.5) { return false; }
    iplug::sample** in = mSocketsIn.Get(0)->mConnectedTo->mParentBuffer;
    for (int o = 0; o < mOutputCount; o++) {
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
    for (int i = 0; i < mSocketsIn.GetSize(); i++) {
      if (mSocketsIn.Get(i)->mConnectedTo == nullptr) {
        outputSilence();
        return false;
      }
    }

    /**
     * Check for inputs which are connected to unprocessed nodes
     */
    for (int i = 0; i < mInputCount; i++) {
      if (!mSocketsIn.Get(i)->mConnectedTo->mParentNode->mIsProcessed) {
        // A node isn't ready so return false
        return false;
      }
    }

    /**
     * Check for automation
     */
    if (mIsAutomated) {
      for (int i = 0; i < mParameters.GetSize(); i++) {
        Node* n = mParameters.Get(i)->automationDependency;
        if (n != nullptr && !n->mIsProcessed) {
          return false;
        }
      }
    }

    return true;
  }

  void checkIsAutomated() {
    for (int i = 0; i < mParameters.GetSize(); i++) {
      if (mParameters.Get(i)->automationDependency != nullptr) {
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
  virtual void OnChannelsChanged(int p_channels) {
    deleteBuffers();
    mChannelCount = p_channels;
    createBuffers();
  }

  virtual void OnSamplerateChanged(int p_sampleRate) {
    mSampleRate = p_sampleRate;
  }

  /** Called on DAW transport e.g. to clear dsp buffers and cut reverbs */
  virtual void OnTransport() { }

  /** Called from the graph to either signal a change in samplerate/channel count or transport */
  virtual void OnReset(int p_sampleRate, int p_channels) {
    bool isTransport = true;
    if (p_sampleRate != mSampleRate) {
      OnSamplerateChanged(p_sampleRate);
      isTransport = false;
    }
    if (p_channels != mChannelCount) {
      OnChannelsChanged(p_channels);
      isTransport = false;
    }
    if (isTransport) {
      OnTransport();
    }
  }

  /**
   * Connects a given socket to a input at a given index of this node
   */
  virtual void connectInput(NodeSocket* out, int inputNumber = 0) {
    NodeSocket* inSocket = mSocketsIn.Get(inputNumber);
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
    ParameterCoupling* p = mParameters.Get(index);
    if (p != nullptr) {
      //if (p->automationDependency != nullptr) {
      //  // Get rid of the old automation if there was one
      //  p->automationDependency->removeAutomationTarget(p);
      //}
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
   * This is for nodes that provide can provide automation for other nodes
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
    mParameters.Add(new ParameterCoupling(
      "Bypass", &mByPassed, 0.0, 0.0, 1.0, 1
    ));
  }

  /** Generic function to call when the node can switch between mono/stereo */
  void addStereoParam(ParameterCoupling* p = nullptr) {
    if (p == nullptr) {
      p = new ParameterCoupling(
        "Stereo", &mStereo, 1.0, 0.0, 1.0, 1
      );
    }
    mParameters.Add(p);
  }




  /**                 UI STUFF                */


  /**
   * Generic setup of the parameters to get something on the screen
   */
  virtual void setupUi(iplug::igraphics::IGraphics* pGrahics) {

    mUi = new NodeUi(NodeUiParam {
      mBus, pGrahics, 300, 300, &mX, &mY,
      &mParameters, &mSocketsIn, &mSocketsOut, this
    });
    mUi->setColor(IColor(255, 100, 100, 100));
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    mUiReady = true;
  }

  /**
   * Cleans up the IControls for all the parameters
   */
  virtual void cleanupUi(iplug::igraphics::IGraphics* pGrahics) {
    /**
     * The param value gets only synced to the dsp value when the node is processed
     * If the node is not connected this won't happen, so always do the update when the
     * Gui window is closed just in case
     */
    for (int i = 0; i < mParameters.GetSize(); i++) {
      mParameters.Get(i)->update();
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

