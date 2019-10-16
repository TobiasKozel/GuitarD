#pragma once

#include <algorithm>
#include "IPlugConstants.h"
#include "IGraphics.h"
#include "src/logger.h"
#include "src/constants.h"
#include "src/graph/ui/NodeUi.h"
#include "src/graph/misc/NodeSocket.h"
#include "src/graph/misc/ParameterCoupling.h"

class Node {
protected:
  bool uiReady;
public:
  // blah, blah not the point of oop but this type name is used to serialize the node
  const char* type;

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

  /**
   * The constructor doesn't take any parameters since it can be instanciated from the NodeList
   */
  Node() {
    type = DefaultNodeName;
    outputs = nullptr;
  };

  /**
   * This is basically a delayed constructor with the only disadvatage: derived methods have to have the same parameters
   */
  virtual void setup(int p_samplerate = 48000, int p_maxBuffer = 512, int p_channles = 2, int p_inputs = 1, int p_outputs = 1) {
    samplerate = p_samplerate;
    maxBuffer = p_maxBuffer;
    inputCount = p_inputs;
    outputCount = p_outputs;
    isProcessed = false;
    channelCount = p_channles;
    uiReady = false;
    X = Y = 0;

    // A derived node might have already setup a custom output buffer
    // Setup the output buffer only if there's none
    if (outputs == nullptr) {
      outputs = new iplug::sample** [p_outputs];
      for (int i = 0; i < p_outputs; i++) {
        outputs[i] = new iplug::sample* [p_channles];
        for (int c = 0; c < p_channles; c++) {
          outputs[i][c] = new iplug::sample[p_maxBuffer];
        }
      }
    }

    // Setup the sockets for the node connections
    for (int i = 0; i < inputCount; i++) {
      NodeSocket* in = new NodeSocket(i);
      inSockets.Add(in);
    }

    for (int i = 0; i < outputCount; i++) {
      NodeSocket* out = new NodeSocket(i, this, outputs[i]);
      outSockets.Add(out);
    }
  }


  virtual ~Node() {
    // clean up the output buffers
    if (outputs != nullptr) {
      for (int i = 0; i < outputCount; i++) {
        for (int c = 0; c < channelCount; c++) {
          delete outputs[i][c];
        }
        delete outputs[i];
      }
      delete outputs;
    }
    
    if (uiReady || mUi != nullptr) {
      WDBGMSG("Warning, UI of node was not cleaned up!\n");
    }

    parameters.Empty(true);
    inSockets.Empty(true);
    outSockets.Empty(true);
  }

  virtual bool inputsReady() {
    for (int i = 0; i < inputCount; i++) {
      if (!inSockets.Get(i)->connectedNode->isProcessed) {
        return false;
      }
    }
    return true;
  }

  virtual void ProcessBlock(int nFrames) = 0;

  virtual void connectInput(NodeSocket* out, int inputNumber = 0) {
    if (inputNumber < inputCount) {
      if (out->isInput) {
        WDBGMSG("Trying to connect an input to an input!");
        assert(false);
        return;
      }
      Node* outNode = out->connectedNode;
      NodeSocket* inSocket = inSockets.Get(inputNumber);
      inSocket->connectedNode = outNode;
      inSocket->buffer = outNode->outputs[out->ownIndex];
      inSocket->connectedBufferIndex = inputNumber;
    }
  }

  virtual void disconnectInput(int inputNumber = 0) {
    if (inputNumber < inputCount) {
      NodeSocket* inSocket = inSockets.Get(inputNumber);
      inSocket->connectedNode = nullptr;
      inSocket->buffer = nullptr;
      inSocket->connectedBufferIndex = -1;
    }
  }

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
      &outSockets
    });
    pGrahics->AttachControl(mUi);

    //for (int i = 0; i < inputCount; i++) {
    //  inSockets[i] = new NodeSocket(pGrahics, "", X + 20, Y + i * 40, i, false, [](int connectedTo) {
    //  });
    //  pGrahics->AttachControl(inSockets[i]);
    //}
    //for (int i = 0; i < outputCount; i++) {
    //  outSockets[i] = new NodeSocket(pGrahics, "", X + 200, Y + i * 40, i, true, [](int connectedTo) {
    //  });
    //  pGrahics->AttachControl(outSockets[i]);
    //}

    uiReady = true;
  }

  virtual void cleanupUi(iplug::igraphics::IGraphics* pGrahics) {

    if (mUi != nullptr) {
      pGrahics->RemoveControl(mUi, true);
      mUi = nullptr;
    }

    //for (int i = 0; i < inputCount; i++) {
    //  pGrahics->RemoveControl(inSockets[i], true);
    //}
    //for (int i = 0; i < outputCount; i++) {
    //  pGrahics->RemoveControl(outSockets[i], true);
    //}
    uiReady = false;
  }

  virtual void layoutChanged() {

  }


};

