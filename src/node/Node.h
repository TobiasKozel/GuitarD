#pragma once

#include <algorithm>
#include "IPlugConstants.h"
#include "IGraphics.h"
#include "src/misc/constants.h"
#include "src/node/NodeUi.h"
#include "src/node/NodeSocket.h"
#include "src/parameter/ParameterCoupling.h"
#include "src/misc/MessageBus.h"

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

  bool mByPassed;

  /**
   * The constructor doesn't take any parameters since it can be instanciated from the NodeList
   */
  Node() {
    mUi = nullptr;
    outputs = nullptr;
    mByPassed = false;
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
      in->X = X - 100;
      in->Y = Y + (i * 50);
      inSockets.Add(in);
    }

    for (int i = 0; i < outputCount; i++) {
      NodeSocket* out = new NodeSocket(i, this, outputs[i]);
      out->X = X + 100;
      out->Y = Y + (i * 50);
      outSockets.Add(out);
    }
  }

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

  virtual ~Node() {
    deleteBuffers();
    
    if (uiReady || mUi != nullptr) {
      WDBGMSG("Warning, UI of node was not cleaned up!\n");
    }

    parameters.Empty(true);
    inSockets.Empty(true);
    outSockets.Empty(true);
  }

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

  bool byPass() {
    if (!mByPassed) { return false; }
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

  /**
   * Check where the node is able to process a block
   */
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

  virtual void BlockStart() {
    isProcessed = false;
  }

  virtual void ProcessBlock(int nFrames) = 0;

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

  /**
   * Generic setup of the parameters to get something on the screen
   */
  virtual void setupUi(iplug::igraphics::IGraphics* pGrahics) {

    mUi = new NodeUi(NodeUiParam {
      pGrahics,
      PNGGENERICBG_FN,
      &X, &Y, &mByPassed,
      &parameters,
      &inSockets,
      &outSockets,
      this
    });
    pGrahics->AttachControl(mUi);
    mUi->setUp();
    uiReady = true;
  }

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

  virtual void layoutChanged() { }

  virtual void channelsChanged(int p_channels) {
    if (outputs != nullptr) {
      deleteBuffers();
    }
    outputs = new iplug::sample * *[outputCount];
    for (int i = 0; i < outputCount; i++) {
      outputs[i] = new iplug::sample * [p_channels];
      for (int c = 0; c < p_channels; c++) {
        outputs[i][c] = new iplug::sample[maxBuffer];
      }
    }
    channelCount = p_channels;
  }

  virtual void samplerateChanged(int p_sampleRate) {
    samplerate = p_sampleRate;
  }

  virtual void OnReset(int p_sampleRate, int p_channels) {
    if (p_channels != channelCount) {
      channelsChanged(p_channels);
    }
    if (p_sampleRate != samplerate) {
      samplerateChanged(p_sampleRate);
    }
  }
};
