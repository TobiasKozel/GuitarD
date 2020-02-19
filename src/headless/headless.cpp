/**
 * This is supposed to be the headless version of the plugin
 * There's no GUI and all of the IPlug components are replaced
 */
#pragma once
#define GUITARD_HEADLESS
#define WDL_RESAMPLE_TYPE float
#define FLOATCONV

#include "soundwoofer/soundwoofer.h"
#include "src/types/types.h"
#include "src/graph/Graph.h"
#include "src/nodes/RegisterNodes.h"
#include "src/misc/MessageBus.h"
#include "src/parameter/ParameterManager.h"

namespace guitard {
  class GuitarDHeadless {
    MessageBus::Bus mBus;
    ParameterManager mParamManager;
    Graph mGraph;
    bool mReady = false;
  public:
    GuitarDHeadless() : mParamManager(&mBus), mGraph(&mBus, &mParamManager) {
      String homeDir;
#ifdef unix
      homeDir = getenv("HOME");
#elif defined(_WIN32)
      homeDir = getenv("HOMEDRIVE");
      homeDir.append(getenv("HOMEPATH"));
#endif
      printf("\n%s\n", homeDir.c_str());
      soundwoofer::setup::setPluginName("GuitarD");
      HOME_PATH = homeDir;
      soundwoofer::setup::setHomeDirectory(homeDir.c_str());
    }

    /**
     * Needs to be called before processing can start to set sample rate and channel config
     */
    void setConfig(int samplerate, int outChannels, int inChannels) {
      if (samplerate > 0 && outChannels > 0 && inChannels > 0) {
        mGraph.OnReset(samplerate, outChannels, inChannels);
        mReady = true;
      }
      else {
        mReady = false;
      }
    }

    void process(sample** in, sample** out, int samples) {
      if (mReady) {
        mGraph.ProcessBlock(in, out, samples);
      }
    }

    /**
     * Resets the plugin (kills reverb tails etc)
     */
    void reset() {
      mGraph.OnTransport();
    }

    /**
     * Takes a value from 0 to 1 to control the parameter
     */
    void setParam(int paramIndex, sample value) {
      ParameterCoupling* couple = mParamManager.getCoupling(paramIndex);
      if (couple != nullptr) {
        couple->setFromNormalized(value);
      }
    }

    /**
     * Provide a json to load, make sure it's null terminated
     * Will block the thread until it's loaded and skip processing
     */
    void load(const char* data) {
      mGraph.deserialize(data);
    }
  };
}

int main() {
  guitard::NodeList::registerNodes();
  const int size = 128;
  const int channels = 2;
  guitard::sample* in[channels];
  guitard::sample* out[channels];
  for (int i = 0; i < channels; i++) {
    in[i] = new guitard::sample[size];
    out[i] = new guitard::sample[size];
    memset(in[i],0, size * sizeof(guitard::sample));
  }
  guitard::GuitarDHeadless headless;
  headless.load("{\"height\":550,\"input\":{\"gain\":1.0,\"position\":[450.0,275.0]},\"nodes\":[{\"idx\":0,\"inputs\":[[-1,0]],\"parameters\":[{\"automation\":-1,\"idx\":28,\"name\":\"Bypass\",\"value\":0.0},{\"automation\":-1,\"idx\":29,\"name\":\"Attack\",\"value\":0.01},{\"automation\":-1,\"idx\":30,\"name\":\"Hold\",\"value\":0.1},{\"automation\":-1,\"idx\":31,\"name\":\"Release\",\"value\":0.1},{\"automation\":-1,\"idx\":32,\"name\":\"Threshold\",\"value\":-56.667046440006416}],\"position\":[787.966796875,255.9927978515625],\"type\":\"SimpleGateNode\"},{\"idx\":1,\"inputs\":[[0,0]],\"parameters\":[{\"automation\":-1,\"idx\":6,\"name\":\"Bypass\",\"value\":0.0},{\"automation\":-1,\"idx\":7,\"name\":\"f1\",\"value\":300.0},{\"automation\":-1,\"idx\":8,\"name\":\"f2\",\"value\":3000.0},{\"automation\":-1,\"idx\":9,\"name\":\"highF\",\"value\":20.0},{\"automation\":-1,\"idx\":10,\"name\":\"highQ\",\"value\":1.0},{\"automation\":-1,\"idx\":11,\"name\":\"lowF\",\"value\":293.5643230129127},{\"automation\":-1,\"idx\":12,\"name\":\"lowQ\",\"value\":1.0},{\"automation\":-1,\"idx\":13,\"name\":\"peak1\",\"value\":0.0},{\"automation\":-1,\"idx\":14,\"name\":\"peak2\",\"value\":0.0},{\"automation\":-1,\"idx\":15,\"name\":\"q1\",\"value\":300.0},{\"automation\":-1,\"idx\":16,\"name\":\"q2\",\"value\":500.0}],\"position\":[1140.18798828125,-275.11834716796875],\"type\":\"ParametricEqNode\"},{\"idx\":2,\"inputs\":[[0,0]],\"parameters\":[{\"automation\":-1,\"idx\":17,\"name\":\"Bypass\",\"value\":0.0},{\"automation\":-1,\"idx\":18,\"name\":\"f1\",\"value\":2756.065898675902},{\"automation\":-1,\"idx\":19,\"name\":\"f2\",\"value\":7114.05596785445},{\"automation\":-1,\"idx\":20,\"name\":\"highF\",\"value\":958.5262639984862},{\"automation\":-1,\"idx\":21,\"name\":\"highQ\",\"value\":1.0},{\"automation\":-1,\"idx\":22,\"name\":\"lowF\",\"value\":5220.329715885276},{\"automation\":-1,\"idx\":23,\"name\":\"lowQ\",\"value\":1.0},{\"automation\":-1,\"idx\":24,\"name\":\"peak1\",\"value\":16.666931152343693},{\"automation\":-1,\"idx\":25,\"name\":\"peak2\",\"value\":0.0},{\"automation\":-1,\"idx\":26,\"name\":\"q1\",\"value\":1096.3069586323215},{\"automation\":-1,\"idx\":27,\"name\":\"q2\",\"value\":500.0}],\"position\":[1231.29931640625,291.54833984375],\"type\":\"ParametricEqNode\"},{\"idx\":3,\"inputs\":[[1,0]],\"parameters\":[{\"automation\":-1,\"idx\":2,\"name\":\"Bypass\",\"value\":0.0},{\"automation\":-1,\"idx\":254,\"name\":\"Bass\",\"value\":0.0},{\"automation\":-1,\"idx\":3,\"name\":\"Drive\",\"value\":0.8101857844697393},{\"automation\":-1,\"idx\":4,\"name\":\"Offset\",\"value\":0.0},{\"automation\":-1,\"idx\":5,\"name\":\"Post gain\",\"value\":0.5}],\"position\":[1492.40966796875,-132.89614868164063],\"type\":\"SimpleDriveNode\"},{\"idx\":4,\"inputs\":[[2,0]],\"parameters\":[{\"automation\":-1,\"idx\":33,\"name\":\"Bypass\",\"value\":0.0},{\"automation\":-1,\"idx\":226,\"name\":\"Bass\",\"value\":0.0},{\"automation\":-1,\"idx\":34,\"name\":\"Drive\",\"value\":0.7222234090169266},{\"automation\":-1,\"idx\":35,\"name\":\"Offset\",\"value\":0.0},{\"automation\":-1,\"idx\":36,\"name\":\"Post gain\",\"value\":0.5}],\"position\":[1700.188232421875,268.2149658203125],\"type\":\"SimpleDriveNode\"},{\"idx\":5,\"inputs\":[[3,0]],\"parameters\":[{\"automation\":-1,\"idx\":40,\"name\":\"Bypass\",\"value\":1.0},{\"automation\":-1,\"idx\":41,\"name\":\"Left\",\"value\":1.0},{\"automation\":-1,\"idx\":42,\"name\":\"Left/Right Offset\",\"value\":0.0},{\"automation\":-1,\"idx\":43,\"name\":\"Phase Fine\",\"value\":0.002222229003906251},{\"automation\":-1,\"idx\":44,\"name\":\"Phase Rough\",\"value\":0.0},{\"automation\":-1,\"idx\":45,\"name\":\"Right\",\"value\":1.0}],\"position\":[1827.96533203125,-184.00732421875],\"type\":\"PhaseToolNode\"},{\"idx\":6,\"inputs\":[[5,0],[4,0]],\"parameters\":[{\"automation\":-1,\"idx\":37,\"name\":\"PAN 1\",\"value\":0.5925902048746738},{\"automation\":-1,\"idx\":38,\"name\":\"PAN 2\",\"value\":-0.24073779259348993},{\"automation\":-1,\"idx\":39,\"name\":\"MIX\",\"value\":0.5601895650227855},{\"automation\":-1,\"idx\":-1,\"name\":\"Add mode\",\"value\":0.0}],\"position\":[2260.1875,-2.896209716796875],\"type\":\"CombineNode\"},{\"idx\":7,\"inputs\":[[6,0]],\"parameters\":[{\"automation\":-1,\"idx\":66,\"name\":\"Bypass\",\"value\":1.0},{\"automation\":-1,\"idx\":67,\"name\":\"Left\",\"value\":1.0},{\"automation\":-1,\"idx\":68,\"name\":\"Left/Right Offset\",\"value\":0.2822188125442191},{\"automation\":-1,\"idx\":69,\"name\":\"Phase Fine\",\"value\":0.0},{\"automation\":-1,\"idx\":70,\"name\":\"Phase Rough\",\"value\":0.0},{\"automation\":-1,\"idx\":71,\"name\":\"Right\",\"value\":1.0}],\"position\":[2573.52099609375,70.43719482421875],\"type\":\"PhaseToolNode\"},{\"idx\":8,\"inputs\":[[7,0]],\"parameters\":[{\"automation\":-1,\"idx\":46,\"name\":\"Bypass\",\"value\":0.0},{\"automation\":-1,\"idx\":47,\"name\":\"f1\",\"value\":1453.859992225187},{\"automation\":-1,\"idx\":48,\"name\":\"f2\",\"value\":7237.61617399968},{\"automation\":-1,\"idx\":49,\"name\":\"highF\",\"value\":123.79204622206827},{\"automation\":-1,\"idx\":50,\"name\":\"highQ\",\"value\":1.0},{\"automation\":-1,\"idx\":51,\"name\":\"lowF\",\"value\":11989.716990549268},{\"automation\":-1,\"idx\":52,\"name\":\"lowQ\",\"value\":1.0},{\"automation\":-1,\"idx\":53,\"name\":\"peak1\",\"value\":9.629748686208316},{\"automation\":-1,\"idx\":54,\"name\":\"peak2\",\"value\":13.703727816308977},{\"automation\":-1,\"idx\":55,\"name\":\"q1\",\"value\":3040.768432617183},{\"automation\":-1,\"idx\":56,\"name\":\"q2\",\"value\":2627.438151041663}],\"position\":[2877.96533203125,13.770477294921875],\"type\":\"ParametricEqNode\"},{\"idx\":9,\"inputs\":[[8,0]],\"parameters\":[{\"automation\":-1,\"idx\":72,\"name\":\"Bypass\",\"value\":1.0},{\"automation\":-1,\"idx\":73,\"name\":\"Panning\",\"value\":0.0},{\"automation\":-1,\"idx\":74,\"name\":\"Post panning\",\"value\":1.0},{\"automation\":-1,\"idx\":75,\"name\":\"Width\",\"value\":0.46296132405599033}],\"position\":[3224.63232421875,-128.45184326171875],\"type\":\"StereoToolNode\"},{\"customIR\":false,\"idx\":10,\"inputs\":[[9,0]],\"irName\":\"Clean\",\"parameters\":[{\"automation\":-1,\"idx\":0,\"name\":\"Bypass\",\"value\":0.0},{\"automation\":-1,\"idx\":1,\"name\":\"Stereo\",\"value\":1.0}],\"path\":\"\",\"position\":[3616.966796875,-167.69879150390625],\"type\":\"SimpleCabNode\"},{\"idx\":11,\"inputs\":[[10,0]],\"parameters\":[{\"automation\":-1,\"idx\":57,\"name\":\"Bypass\",\"value\":0.0},{\"automation\":-1,\"idx\":58,\"name\":\"Band1\",\"value\":138.87233960286622},{\"automation\":-1,\"idx\":59,\"name\":\"Band2\",\"value\":8000.0},{\"automation\":-1,\"idx\":60,\"name\":\"Decay Band1\",\"value\":0.01},{\"automation\":-1,\"idx\":61,\"name\":\"Decay Band2\",\"value\":0.19500056457519555},{\"automation\":-1,\"idx\":200,\"name\":\"Mix\",\"value\":1.0},{\"automation\":-1,\"idx\":199,\"name\":\"Predelay\",\"value\":0.0}],\"position\":[3935.74365234375,10.437286376953125],\"type\":\"SimpleReverbNode\"},{\"idx\":12,\"inputs\":[[10,0],[11,0]],\"parameters\":[{\"automation\":-1,\"idx\":63,\"name\":\"PAN 1\",\"value\":0.0},{\"automation\":-1,\"idx\":64,\"name\":\"PAN 2\",\"value\":0.1759264628092445},{\"automation\":-1,\"idx\":65,\"name\":\"MIX\",\"value\":0.23148276482066815},{\"automation\":-1,\"idx\":-1,\"name\":\"Add mode\",\"value\":0.0}],\"position\":[4192.41064453125,-294.0072937011719],\"type\":\"CombineNode\"},{\"idx\":13,\"inputs\":[[12,0]],\"parameters\":[{\"automation\":-1,\"idx\":62,\"name\":\"Bypass\",\"value\":0.0},{\"automation\":-1,\"idx\":76,\"name\":\"Gain\",\"value\":-9.208333692513408}],\"position\":[4527.1875,-310.4517822265625],\"type\":\"AutoGainNode\"}],\"output\":{\"gain\":1.0,\"inputs\":[[13,0]],\"position\":[4934.1875,-291.34954833984375]},\"scale\":1.0,\"version\":65536,\"width\":900}");
  headless.setConfig(48000, channels, channels);
  while (true) {
    headless.process(in, out, size);
  }
  return 0;
}