#pragma once

#include "../Node.h"
#define FAUSTFLOAT sample

namespace guitard {
  namespace FaustGenerated {
    /**
     * Passed to the generated faust code to gather the copyright info
     */
    struct Meta {
      String result = "\nDSP Code generated using Grame Faust\n";
      void declare(const char* key, const char* value) {
        result += String(key) + ": " + String(value) + "\n";
      };
    };

    /**
     * This is a shim to collect pointers to all the properties/parameters from the faust DSP code
     */
    struct UI {
      const char* name;
      Node* node = nullptr;
      UI(Node* n) {
        node = n;
        name = GUITARD_DEFAULT_NODE_NAME;
      }

      void openVerticalBox(const char* key) {
        // NOTE This only is the name of the module if it has one box!
        if (name != GUITARD_DEFAULT_NODE_NAME) {
          WDBGMSG("openVerticalBox called multiple times. The node Type might be wrong!");
          assert(false);
        }
        name = key;
      };

      static void openHorizontalBox(const char* key) {};
      static void closeBox() {};
      static void declare(FAUSTFLOAT*, const char*, const char*) {};

      void addHorizontalSlider(
          const char* name, FAUSTFLOAT* prop, FAUSTFLOAT pDefault,
          FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT stepSize
      ) const {
        node->addParameter(name, prop, pDefault, min, max, stepSize);
      }

      void addVerticalSlider(
          const char* name, FAUSTFLOAT* prop, FAUSTFLOAT pDefault,
          FAUSTFLOAT min, FAUSTFLOAT max, FAUSTFLOAT stepSize
      ) const {
        addHorizontalSlider(name, prop, pDefault, min, max, stepSize);
      }

      void addCheckButton(const char* name, FAUSTFLOAT* prop) const {
        addHorizontalSlider(name, prop, 0, 0, 1, 1);
      }

      void addVerticalBargraph(const char* name, FAUSTFLOAT* prop, FAUSTFLOAT min, FAUSTFLOAT max) const {
        node->addMeter(name, prop, min, max);
      };

      void addHorizontalBargraph(const char* name, FAUSTFLOAT* prop, FAUSTFLOAT min, FAUSTFLOAT max) const {
        addVerticalBargraph(name, prop, min, max);
      };
    };


    /**
     * The faust DSP code will derive from this
     */
    class FaustHeadlessDsp : public Node {
      /**
       * The realigned output buffers the faust dsp will write to
       */
      FAUSTFLOAT** mBuffersOutAligned = nullptr;
      FAUSTFLOAT** mBuffersInAligned = nullptr;

    public:
      // These three will be overridden by the generated faust code
      virtual void init(int samplingFreq) = 0;
      virtual void buildUserInterface(UI* ui_interface) = 0;
      virtual void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) = 0;
      virtual int getNumInputs() = 0;
      virtual int getNumOutputs() = 0;
      virtual void instanceConstants(int samplingFreq) = 0;
      virtual void instanceClear() = 0;
      virtual void metadata(Meta* m) = 0;

      void setup(
          const int pSamplerate, const int pMaxBuffer = GUITARD_MAX_BUFFER,
          int pInputs = 1, int pOutputs = 1, const int pChannels = 2
      ) override {

        Node::setup(
          pSamplerate, pMaxBuffer, getNumInputs() / pChannels,
          getNumOutputs() / pChannels, pChannels
        );

        addByPassParam();
        /**
         * This will use the UI shim to create ParameterCouplings between the faust dsp and iplug iControls
         * However they will not be registered to the daw yet, since loading a preset will need them to claim
         * the right ones so the automation will affect the correct parameters
         */
        UI faustUi(this);

        buildUserInterface(&faustUi);
        init(pSamplerate);

        if (mInfo->name == GUITARD_DEFAULT_NODE_NAME) { // If a name wasn't set from outside, use the from faust
          mInfo->name = faustUi.name;
        }

        const int perColumn = 2;
        // const int columns = ceil(shared.parameterCount / static_cast<float>(perColumn));

        for (int i = 0, pos = 0; i < mParameterCount; i++) {
          const int column = pos / perColumn;
          ParameterCoupling* p = &mParameters[i];
          if (strncmp(p->name, "Bypass", 32) == 0) {
            continue;
          }
          //p->pos.x = column * 60 + 50 - mDimensions.x * 0.5;
          //p->pos.y = 60 * (pos % perColumn) - 40;
          pos++;
        }
      }

      /**
       * Faust code can't handle changing channel counts as easily
       */
      void OnChannelsChanged(const int pChannels) override {
        if (pChannels != 2) {
          WDBGMSG("Fuast code only works with 2 channels!");
        }
        Node::OnChannelsChanged(pChannels);
      }

      void OnSamplerateChanged(const int pSamplerate) override {
        Node::OnSamplerateChanged(pSamplerate);
        instanceConstants(mSampleRate);
      }

      void OnTransport() override {
        instanceClear();
      }

      /**
       * Delete the realigned buffers
       */
      void deleteBuffers() override {
        Node::deleteBuffers();
        delete[] mBuffersOutAligned;
        delete[] mBuffersInAligned;
      }

      /**
       * Faust uses a different way to handle channels
       * Nodes use stereo pairs, while faust channels will all be in a single array
       * this doesn't work for inputs as easily though since inputs are
       * only known after a node is connected
       */
      void createBuffers() override {
        Node::createBuffers();
        mBuffersOutAligned = new FAUSTFLOAT * [mOutputCount * mChannelCount];
        for (int i = 0; i < mOutputCount; i++) {
          for (int c = 0; c < mChannelCount; c++) {
            mBuffersOutAligned[i * mChannelCount + c] = mSocketsOut[i].mBuffer[c];
          }
        }
        mBuffersInAligned = new FAUSTFLOAT * [mInputCount * mChannelCount];
        instanceClear();
      }

      void OnConnectionsChanged() override {
        Node::OnConnectionsChanged();
        for (int i = 0; i < mInputCount; i++) {
          for (int c = 0; c < mChannelCount; c++) {
            mBuffersInAligned[i * mChannelCount + c] = mSocketsIn[i].mBuffer[c];
          }
        }
      }

      void enableOversampling() override {
        Node::enableOversampling();
        mOverSampler->mProc = [&](sample** inputs, sample** outputs, int nFrames) {
          this->compute(nFrames, inputs, outputs);
        };
      }

      /**
       * The faust uses a fairly similar way of processing blocks,
       * so this just wraps the call and updates the parameters
       */
      void ProcessBlock(const int nFrames) override {
        if (byPass()) { return; }
        for (int i = 1; i < mParameterCount; i++) {
          mParameters[i].update();
        }
        if (mOverSampler != nullptr) {
          updateOversampling();
          mOverSampler->process(mBuffersInAligned, mBuffersOutAligned, nFrames);
        }
        else {
          compute(nFrames, mBuffersInAligned, mBuffersOutAligned);
        }
      }

      /**
       * Retrieve the copyright info from the faust generated code
       */
      String getLicense() override {
        Meta m;
        metadata(&m);
        return m.result;
      }
    };

  }
}