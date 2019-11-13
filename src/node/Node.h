#pragma once

#include <algorithm>
#include "IPlugConstants.h"
#include "IGraphics.h"
#include "src/misc/constants.h"
#include "src/node/NodeUi.h"
#include "src/node/NodeSocket.h"
#include "src/parameter/ParameterCoupling.h"
#include "src/misc/MessageBus.h"
#include "src/ui/theme.h"

/**
 * Virtual class which all nodes will derive from
 */
class Node {
protected:
  bool uiReady;
public:
  std::string type;

  WDL_PtrList<ParameterCoupling> parameters;
  // The dsp will get the data from the buffer inside the socket
  WDL_PtrList<NodeSocket> inSockets;
  WDL_PtrList<NodeSocket> outSockets;

  // The dsp will write the result here and it will be exposed to other nodes over the NodeSocket
  iplug::sample*** outputs;
  int inputCount;
  int outputCount;
  NodeUi* mUi;
  bool isProcessed;
  
  float X;
  float Y;
  float rotation;

  int samplerate;
  int channelCount;
  int maxBuffer;
  int mLastBlockSize;

  double mByPassed;
  double mStereo;

  /**
   * The constructor doesn't take any parameters since it can be instanciated from the NodeList
   */
  Node() {
    mUi = nullptr;
    outputs = nullptr;
    mByPassed = 0;
    mStereo = 1;
    X = Y = 0;
    rotation = 0;
  };

  /**
   * This is basically a delayed constructor with the only disadvatage: derived methods have to have the same parameters
   */
  virtual void setup(int p_samplerate = 48000, int p_maxBuffer = 512, int p_channles = 2, int p_inputs = 1, int p_outputs = 1) {
    samplerate = 0;
    channelCount = 0;
    maxBuffer = p_maxBuffer;
    inputCount = p_inputs;
    outputCount = p_outputs;
    isProcessed = false;
    uiReady = false;
    OnReset(p_samplerate, p_channles);



    // Setup the sockets for the node connections
    for (int i = 0; i < inputCount; i++) {
      NodeSocket* in = new NodeSocket(i, this);
      inSockets.Add(in);
    }

    for (int i = 0; i < outputCount; i++) {
      NodeSocket* out = new NodeSocket(i, this, outputs[i]);
      outSockets.Add(out);
    }
  }

  /** create all the needed buffers for the dsp */
  virtual void createBuffers() {
    if (outputs != nullptr) {
      WDBGMSG("Trying to create a new dsp buffer without cleanung up the old one");
      assert(true);
    }
    outputs = new iplug::sample * *[outputCount];
    for (int i = 0; i < outputCount; i++) {
      outputs[i] = new iplug::sample * [channelCount];
      for (int c = 0; c < channelCount; c++) {
        outputs[i][c] = new iplug::sample[maxBuffer];
      }
    }
  }

  /** Delets all the allocated buffers */
  virtual void deleteBuffers() {
    if (outputs != nullptr) {
      for (int i = 0; i < outputCount; i++) {
        for (int c = 0; c < channelCount; c++) {
          delete outputs[i][c];
        }
        delete outputs[i];
      }
      delete outputs;
      outputs = nullptr;
    }
  }

  /** Should do all the required cleanup */
  virtual ~Node() {
    deleteBuffers();
    
    if (uiReady || mUi != nullptr) {
      WDBGMSG("Warning, UI of node was not cleaned up!\n");
    }

    parameters.Empty(true);
    inSockets.Empty(true);
    outSockets.Empty(true);
  }

  /** Will fill all the output buffers with silence */
  void outputSilence() {
    for (int o = 0; o < outputCount; o++) {
      for (int c = 0; c < channelCount; c++) {
        for (int i = 0; i < maxBuffer; i++) {
          outputs[o][c][i] = 0;
        }
      }
    }
    isProcessed = true;
  }

  /** Will return true if the node is bypassed and also do the bypassing of buffers */
  bool byPass() {
    // The first param will always be bypass
    parameters.Get(0)->update();
    if (mByPassed < 0.5) { return false; }
    iplug::sample** in = inSockets.Get(0)->connectedTo->parentBuffer;
    for (int o = 0; o < outputCount; o++) {
      for (int c = 0; c < channelCount; c++) {
        for (int i = 0; i < maxBuffer; i++) {
          outputs[o][c][i] = in[c][i];
        }
      }
    }
    isProcessed = true;
    return true;
  }

  /** Check where the node is able to process a block */
  virtual bool inputsReady() {
    /**
     * If one input isn't connected, skip the processing and output silence
     * If that's not desired, this function has to be overidden
     */
    for (int i = 0; i < inSockets.GetSize(); i++) {
      if (inSockets.Get(i)->connectedTo == nullptr) {
        outputSilence();
        return false;
      }
    }

    for (int i = 0; i < inputCount; i++) {
      if (!inSockets.Get(i)->connectedTo->parentNode->isProcessed) {
        // A node isn't ready so return false
        return false;
      }
    }
    return true;
  }

  /** Main Processing, only takes a blocksize since it knows its inputs */
  virtual void ProcessBlock(int nFrames) = 0;





  /**                  Signals from outside                  */

  /** Signals a new audio block is about to processed */
  virtual void BlockStart() {
    isProcessed = false;
  }

  /** Called if the daw changed the channelcount*/
  virtual void OnChannelsChanged(int p_channels) {
    deleteBuffers();
    channelCount = p_channels;
    createBuffers();
  }

  virtual void OnSamplerateChanged(int p_sampleRate) {
    samplerate = p_sampleRate;
  }

  /** Called on DAW transport e.g. to clear dsp buffer and cut reverbs */
  virtual void OnTransport() { }

  /** Called from the graph to either signal a change in samplerate/channel count or transport */
  virtual void OnReset(int p_sampleRate, int p_channels) {
    bool isTransport = true;
    if (p_channels != channelCount) {
      OnChannelsChanged(p_channels);
      isTransport = false;
    }
    if (p_sampleRate != samplerate) {
      OnSamplerateChanged(p_sampleRate);
      isTransport = false;
    }
    if (isTransport) {
      OnTransport();
    }
  }

  /** Connects a given socket to a input at a given index of this node */
  virtual void connectInput(NodeSocket* out, int inputNumber = 0) {
    NodeSocket* inSocket = inSockets.Get(inputNumber);
    if (inSocket != nullptr) {
      if (out == nullptr) {
        inSocket->disconnect();
      }
      else {
        inSocket->connect(out);
      }
    }
  }

  /** Generic function to call when the node can be bypassed*/
  void addByPassParam() {
    parameters.Add(new ParameterCoupling(
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
    parameters.Add(p);
  }




  /**                 UI STUFF                */


  /**
   * Generic setup of the parameters to get something on the screen
   */
  virtual void setupUi(iplug::igraphics::IGraphics* pGrahics) {

    mUi = new NodeUi(NodeUiParam {
      pGrahics,
      PNGGENERICBG_FN,
      &X, &Y,
      &parameters,
      &inSockets,
      &outSockets,
      this
    });
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    uiReady = true;
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
    for (int i = 0; i < parameters.GetSize(); i++) {
      parameters.Get(i)->update();
    }
    if (mUi != nullptr) {
      mUi->cleanUp();
      pGrahics->RemoveControl(mUi, true);
      mUi = nullptr;
    }
    uiReady = false;
  }

  /** Called if the ui is resized TODOG move over to NodeUI */
  virtual void layoutChanged() { }
};

