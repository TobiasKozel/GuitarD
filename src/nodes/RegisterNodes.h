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


namespace NodeList {
  void registerNodes() {
    NodeList::registerNode(NodeList::NodeInfo {
      []() { return new StereoToolNode("StereoToolNode"); },
      "StereoToolNode",
      "Stereo Tool",
      "asd",
      "Tools"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new CombineNode("CombineNode"); },
      "CombineNode",
      "Combine",
      "asd",
      "Tools"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new FeedbackNode("FeedbackNode"); },
      "FeedbackNode",
      "Feedback",
      "asd",
      "Tools"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new PhaseToolNode("PhaseToolNode"); },
      "PhaseToolNode",
      "Phase Tool",
      "asd",
      "Tools"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new EnvelopeNode("EnvelopeNode"); },
      "EnvelopeNode",
      "Envelope Automation Tool",
      "asd",
      "Automation"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new SimpleDriveNode("SimpleDriveNode"); },
      "SimpleDriveNode",
      "Simple Drive",
      "asd",
      "Distortion"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new OverDriveNode("OverDriveNode"); },
        "OverDriveNode",
        "Overdrive",
        "asd",
        "Distortion"
      });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new FuzzNode("FuzzNode"); },
      "FuzzNode",
      "Fuzz",
      "asd",
      "Distortion"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new BitCrusherNode("BitCrusherNode"); },
      "BitCrusherNode",
      "Bitcrusher",
      "asd",
      "Distortion"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new PowerSagNode("PowerSagNode"); },
      "PowerSagNode",
      "Power Sag",
      "asd",
      "Distortion"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new SimpleDelayNode("SimpleDelayNode"); },
      "SimpleDelayNode",
      "Mono Delay",
      "asd",
      "Delays/Reverbs"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new SimpleReverbNode("SimpleReverbNode"); },
      "SimpleReverbNode",
      "Stereo Reverb",
      "asd",
      "Delays/Reverbs"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new SimpleCabNode("SimpleCabNode"); },
      "SimpleCabNode",
      "Simple Cabinet",
      "asd",
      "Cabinets"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new CryBabyNode("CryBabyNode"); },
      "CryBabyNode",
      "Crybaby",
      "asd",
      "Filters"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new ParametricEqNode("ParametricEqNode"); },
      "ParametricEqNode",
      "Parametric Equalizer",
      "asd",
      "Filters"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new SimpleComressorNode("SimpleComressorNode"); },
      "SimpleComressorNode",
      "Simple Compressor",
      "asd",
      "Dynamics"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new SimpleGateNode("SimpleGateNode"); },
      "SimpleGateNode",
      "Simple Gate",
      "asd",
      "Dynamics"
    });

    NodeList::registerNode(NodeList::NodeInfo{
      []() { return new AutoGainNode("AutoGainNode"); },
      "AutoGainNode",
      "Auto Linear Gain",
      "asd",
      "Dynamics"
      });
  }
}