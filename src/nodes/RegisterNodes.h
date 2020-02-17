/**
 * All the nodes need to be included here
 * registerNodes() will be called when the plugin is constructed
 * and will register all nodes in a global list which will
 * be shared across plugin instances if they are in the same process
 */

#pragma once
#include "../misc/NodeList.h"
#include "./stereo_tool/StereoToolNode.h"
#include "./simple_drive/SimpleDriveNode.h"
#include "./simple_delay/SimpleDelayNode.h"
#include "./simple_cab/SimpleCabNode.h"
#include "./crybaby/CryBabyNode.h"
#include "./combine/CombineNode.h"
#include "./feedback/FeedbackNode.h"
#include "./simple_reverb/SimpleReverbNode.h"
#include "./simple_gate/SimpleGateNode.h"
#include "./simple_compressor/SimpleComressorNode.h"
#include "./fuzz/FuzzNode.h"
#include "./bitcrusher/BitCrusherNode.h"
#include "./overdrive/OverDriveNode.h"
#include "./paramerticeq/ParametricEqNode.h"
#include "./phasetool/PhaseToolNode.h"
#include "./envelope/EnvelopeNode.h"
#include "./autogain/AutoGainNode.h"
#include "./powersag/PowerSagNode.h"
#include "./reverse_delay/ReverseDelayNode.h"
#include "./cab_lib/CabLibNode.h"
#include "./split/SplitNode.h"
#include "./band_split/BandSplitNode.h"
#include "./flanger/FlangerNode.h"

namespace guitard {
  namespace NodeList {
    void registerNodes() {
      registerNode(NodeInfo {
        "CombineNode",
        "Combine",
        "asd",
        "Signal Flow",
        [](NodeInfo info) { return new CombineNode(info); }
      });

      registerNode(NodeInfo {
        "SplitNode",
        "Split L/R",
        "asd",
        "Signal Flow",
        [](NodeInfo info) { return new SplitNode(info); }
      });

      registerNode(NodeInfo {
        "BandSplitNode",
        "Band Split",
        "asd",
        "Signal Flow",
        [](NodeInfo info) { return new BandSplitNode(info); }
      });


      registerNode(NodeInfo {
        "StereoToolNode",
        "Stereo Tool",
        "asd",
        "Tools",
        [](NodeInfo info) { return new StereoToolNode(info); }
      });

      registerNode(NodeInfo {
        "FeedbackNode",
        "Feedback",
        "asd",
        "Tools",
        [](NodeInfo info) { return new FeedbackNode(info); }
      });

      registerNode(NodeInfo {
        "PhaseToolNode",
        "Phase Tool",
        "asd",
        "Tools",
        [](NodeInfo info) { return new PhaseToolNode(info); }
      });

      registerNode(NodeInfo {
        "EnvelopeNode",
        "Envelope Automation Tool",
        "asd",
        "Automation",
        [](NodeInfo info) { return new EnvelopeNode(info); }
      });

      registerNode(NodeInfo {
        "SimpleDriveNode",
        "Simple Drive",
        "asd",
        "Distortion",
        [](NodeInfo info) { return new SimpleDriveNode(info); }
      });

      registerNode(NodeInfo {
          "OverDriveNode",
          "Overdrive",
          "asd",
          "Distortion",
        [](NodeInfo info) { return new OverDriveNode(info); }
      });

      registerNode(NodeInfo {
        "FuzzNode",
        "Fuzz",
        "asd",
        "Distortion",
        [](NodeInfo info) { return new FuzzNode(info); }
      });

      registerNode(NodeInfo {
        "BitCrusherNode",
        "Bitcrusher",
        "asd",
        "Distortion",
        [](NodeInfo info) { return new BitCrusherNode(info); }
      });

      registerNode(NodeInfo {
        "PowerSagNode",
        "Power Sag",
        "asd",
        "Distortion",
        [](NodeInfo info) { return new PowerSagNode(info); }
      });

      registerNode(NodeInfo {
        "SimpleDelayNode",
        "Mono Delay",
        "asd",
        "Delays/Reverbs",
        [](NodeInfo info) { return new SimpleDelayNode(info); }
      });

      registerNode(NodeInfo {
        "SimpleReverbNode",
        "Stereo Reverb",
        "asd",
        "Delays/Reverbs",
        [](NodeInfo info) { return new SimpleReverbNode(info); }
      });

      registerNode(NodeInfo {
        "ReverseDelayNode",
        "Reverse Reverb",
        "asd",
        "Delays/Reverbs",
        [](NodeInfo info) { return new ReverseDelayNode(info); }
      });

      registerNode(NodeInfo {
        "SimpleCabNode",
        "Simple Cabinet",
        "asd",
        "Cabinets",
        [](NodeInfo info) { return new SimpleCabNode(info); }
      });

      registerNode(NodeInfo {
        "CabLibNode",
        "Cabinet Library",
        "asd",
        "Cabinets",
        [](NodeInfo info) { return new CabLibNode(info); }
      });

      registerNode(NodeInfo {
        "CryBabyNode",
        "Crybaby",
        "asd",
        "Filters",
        [](NodeInfo info) { return new CryBabyNode(info); }
      });


      registerNode(NodeInfo {
        "FlangerNode",
        "Flanger",
        "asd",
        "Filters",
        [](NodeInfo info) { return new FlangerNode(info); }
      });

      registerNode(NodeInfo {
        "ParametricEqNode",
        "Parametric Equalizer",
        "asd",
        "Filters",
        [](NodeInfo info) { return new ParametricEqNode(info); }
      });

      registerNode(NodeInfo {
        "SimpleComressorNode",
        "Simple Compressor",
        "asd",
        "Dynamics",
        [](NodeInfo info) { return new SimpleComressorNode(info); }
      });

      registerNode(NodeInfo {
        "SimpleGateNode",
        "Simple Gate",
        "asd",
        "Dynamics",
        [](NodeInfo info) { return new SimpleGateNode(info); }
      });

      registerNode(NodeInfo {
        "AutoGainNode",
        "Auto Linear Gain",
        "asd",
        "Dynamics",
        [](NodeInfo info) { return new AutoGainNode(info); }
      });
    }
  }
}