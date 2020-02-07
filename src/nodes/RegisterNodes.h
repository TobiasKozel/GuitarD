/**
 * All the nodes need to be included here
 * registerNodes() will be called when the plugin is constructed
 * and will register all nodes in a global list which will
 * be shared across plugin instances if they are in the same process
 */

#pragma once
#include "src/misc/NodeList.h"
#include "src/nodes/stereo_tool/StereoToolNode.h"
#include "src/nodes/simple_drive/SimpleDriveNode.h"
#include "src/nodes/simple_delay/SimpleDelayNode.h"
#include "src/nodes/simple_cab/SimpleCabNode.h"
#include "src/nodes/crybaby/CryBabyNode.h"
#include "src/nodes/combine/CombineNode.h"
#include "src/nodes/feedback/FeedbackNode.h"
#include "src/nodes/simple_reverb/SimpleReverbNode.h"
#include "src/nodes/simple_gate/SimpleGateNode.h"
#include "src/nodes/simple_compressor/SimpleComressorNode.h"
#include "src/nodes/fuzz/FuzzNode.h"
#include "src/nodes/bitcrusher/BitCrusherNode.h"
#include "src/nodes/overdrive/OverDriveNode.h"
#include "src/nodes/paramerticeq/ParametricEqNode.h"
#include "src/nodes/phasetool/PhaseToolNode.h"
#include "src/nodes/envelope/EnvelopeNode.h"
#include "src/nodes/autogain/AutoGainNode.h"
#include "src/nodes/powersag/PowerSagNode.h"
#include "src/nodes/reverse_delay/ReverseDelayNode.h"
#include "src/nodes/cab_lib/CabLibNode.h"
#include "src/nodes/split/SplitNode.h"
#include "src/nodes/band_split/BandSplitNode.h"
#include "src/nodes/flanger/FlangerNode.h"

namespace guitard {
  namespace NodeList {
    void registerNodes() {
      registerNode(NodeInfo {
        []() { return new CombineNode("CombineNode"); },
        "CombineNode",
        "Combine",
        "asd",
        "Signal Flow"
        });

      registerNode(NodeInfo {
        []() { return new SplitNode("SplitNode"); },
        "SplitNode",
        "Split L/R",
        "asd",
        "Signal Flow"
        });

      registerNode(NodeInfo {
        []() { return new BandSplitNode("BandSplitNode"); },
        "BandSplitNode",
        "Band Split",
        "asd",
        "Signal Flow"
        });


      registerNode(NodeInfo {
        []() { return new StereoToolNode("StereoToolNode"); },
        "StereoToolNode",
        "Stereo Tool",
        "asd",
        "Tools"
        });

      registerNode(NodeInfo {
        []() { return new FeedbackNode("FeedbackNode"); },
        "FeedbackNode",
        "Feedback",
        "asd",
        "Tools"
        });

      registerNode(NodeInfo {
        []() { return new PhaseToolNode("PhaseToolNode"); },
        "PhaseToolNode",
        "Phase Tool",
        "asd",
        "Tools"
        });

      registerNode(NodeInfo {
        []() { return new EnvelopeNode("EnvelopeNode"); },
        "EnvelopeNode",
        "Envelope Automation Tool",
        "asd",
        "Automation"
        });

      registerNode(NodeInfo {
        []() { return new SimpleDriveNode("SimpleDriveNode"); },
        "SimpleDriveNode",
        "Simple Drive",
        "asd",
        "Distortion"
        });

      registerNode(NodeInfo {
        []() { return new OverDriveNode("OverDriveNode"); },
          "OverDriveNode",
          "Overdrive",
          "asd",
          "Distortion"
        });

      registerNode(NodeInfo {
        []() { return new FuzzNode("FuzzNode"); },
        "FuzzNode",
        "Fuzz",
        "asd",
        "Distortion"
        });

      registerNode(NodeInfo {
        []() { return new BitCrusherNode("BitCrusherNode"); },
        "BitCrusherNode",
        "Bitcrusher",
        "asd",
        "Distortion"
        });

      registerNode(NodeInfo {
        []() { return new PowerSagNode("PowerSagNode"); },
        "PowerSagNode",
        "Power Sag",
        "asd",
        "Distortion"
        });

      registerNode(NodeInfo {
        []() { return new SimpleDelayNode("SimpleDelayNode"); },
        "SimpleDelayNode",
        "Mono Delay",
        "asd",
        "Delays/Reverbs"
        });

      registerNode(NodeInfo {
        []() { return new SimpleReverbNode("SimpleReverbNode"); },
        "SimpleReverbNode",
        "Stereo Reverb",
        "asd",
        "Delays/Reverbs"
        });

      registerNode(NodeInfo {
        []() { return new ReverseDelayNode("ReverseDelayNode"); },
        "ReverseDelayNode",
        "Reverse Reverb",
        "asd",
        "Delays/Reverbs"
        });

      registerNode(NodeInfo {
        []() { return new SimpleCabNode("SimpleCabNode"); },
        "SimpleCabNode",
        "Simple Cabinet",
        "asd",
        "Cabinets"
        });

      registerNode(NodeInfo {
        []() { return new CabLibNode("CabLibNode"); },
        "CabLibNode",
        "Cabinet Library",
        "asd",
        "Cabinets"
        });

      registerNode(NodeInfo {
        []() { return new CryBabyNode("CryBabyNode"); },
        "CryBabyNode",
        "Crybaby",
        "asd",
        "Filters"
        });


      registerNode(NodeInfo {
        []() { return new FlangerNode("FlangerNode"); },
        "FlangerNode",
        "Flanger",
        "asd",
        "Filters"
        });

      registerNode(NodeInfo {
        []() { return new ParametricEqNode("ParametricEqNode"); },
        "ParametricEqNode",
        "Parametric Equalizer",
        "asd",
        "Filters"
        });

      registerNode(NodeInfo {
        []() { return new SimpleComressorNode("SimpleComressorNode"); },
        "SimpleComressorNode",
        "Simple Compressor",
        "asd",
        "Dynamics"
        });

      registerNode(NodeInfo {
        []() { return new SimpleGateNode("SimpleGateNode"); },
        "SimpleGateNode",
        "Simple Gate",
        "asd",
        "Dynamics"
        });

      registerNode(NodeInfo {
        []() { return new AutoGainNode("AutoGainNode"); },
        "AutoGainNode",
        "Auto Linear Gain",
        "asd",
        "Dynamics"
        });
    }
  }
}