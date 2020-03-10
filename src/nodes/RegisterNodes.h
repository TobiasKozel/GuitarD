#pragma once
#include "../misc/NodeList.h"

/**
 * Tools
 */
#include "./stereo_tool/StereoToolNode.h"
#include "./phasetool/PhaseToolNode.h"

/**
 * Distortions
 */
#include "./simple_drive/SimpleDriveNode.h"
#include "./bitcrusher/BitCrusherNode.h"
#include "./rectify/RectifyNode.h"
#include "./fuzz/FuzzNode.h"
#include "./overdrive/OverDriveNode.h"
#include "./powersag/PowerSagNode.h"


/**
 * Delay/Reverbs
 */
#include "./simple_delay/SimpleDelayNode.h"
#include "./simple_reverb/SimpleReverbNode.h"

/**
 * Filters
 */
#include "./crybaby/CryBabyNode.h"


/**
 * Routing
 */
#include "./combine/CombineNode.h"
#include "./feedback/FeedbackNode.h"
#include "./split/SplitNode.h"
#include "./band_split/BandSplitNode.h"


//#include "./simple_cab/SimpleCabNode.h"

//#include "./simple_gate/SimpleGateNode.h"
//#include "./simple_compressor/SimpleComressorNode.h"



//#include "./paramerticeq/ParametricEqNode.h"
//#include "./envelope/EnvelopeNode.h"
//#include "./autogain/AutoGainNode.h"
//#include "./reverse_delay/ReverseDelayNode.h"
//#include "./cab_lib/CabLibNode.h"
//#include "./flanger/FlangerNode.h"
//#include "./autowah/AutoWahNode.h"
//#include "./phaser/PhaseNode.h"
//#include "./lfo/LfoNode.h"
//#include "./rectify/RectifyNode.h"
//#include "./transpose/TransposeNode.h"

//registerNode(NodeInfo{
      //registerNode(NodeInfo{
      //  "EnvelopeNode",
      //  "Envelope Automation Tool",
      //  "asd",
      //  "Automation",
      //  "Description",
      //  [](NodeInfo* info) { return new EnvelopeNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "LfoNode",
      //  "LFO Automation Tool",
      //  "asd",
      //  "Automation",
      //  "Description",
      //  [](NodeInfo* info) { return new LfoNode(info); }
      //  });

      //  registerNode(NodeInfo{
      //    "RectifyNode",
      //    "Rectify",
      //    "asd",
      //    "Distortion",
      //    "Description",
      //  [](NodeInfo* info) { return new RectifyNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "FuzzNode",
      //  "Fuzz",
      //  "asd",
      //  "Distortion",
      //  "Description",
      //  [](NodeInfo* info) { return new FuzzNode(info); },
      //  true
      //  });

      //registerNode(NodeInfo{
      //  "BitCrusherNode",
      //  "Bitcrusher",
      //  "asd",
      //  "Distortion",
      //  "Description",
      //  [](NodeInfo* info) { return new BitCrusherNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "SimpleDelayNode",
      //  "Basic Delay",
      //  "asd",
      //  "Delays/Reverbs",
      //  "A very simple delay effect",
      //  [](NodeInfo* info) { return new SimpleDelayNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "ReverseDelayNode",
      //  "Reverse Delay",
      //  "asd",
      //  "Delays/Reverbs",
      //  "Reversed Delay effect (Kinda Clicky)",
      //  [](NodeInfo* info) { return new ReverseDelayNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "SimpleCabNode",
      //  "Simple Cabinet",
      //  "asd",
      //  "Cabinets",
      //  "Provides a few IRs and allow loading .wav IRs up to 10 seconds",
      //  [](NodeInfo* info) { return new SimpleCabNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "CabLibNode",
      //  "Cabinet Library",
      //  "asd",
      //  "Cabinets",
      //  "Allows flipping through IRs quickly. Offline and Online (Soundwoofer) IR database",
      //  [](NodeInfo* info) { return new CabLibNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "AutoWahNode",
      //  "Auto Wah",
      //  "asd",
      //  "Filters",
      //  "Description",
      //  [](NodeInfo* info) { return new AutoWahNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "TransposeNode",
      //  "Transpose",
      //  "asd",
      //  "Filters",
      //  "Description",
      //  [](NodeInfo* info) { return new TransposeNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "PhaserNode",
      //  "Phaser",
      //  "asd",
      //  "Filters",
      //  "Description",
      //  [](NodeInfo* info) { return new PhaserNode(info); }
      //  });


      //registerNode(NodeInfo{
      //  "FlangerNode",
      //  "Flanger",
      //  "asd",
      //  "Filters",
      //  "Description",
      //  [](NodeInfo* info) { return new FlangerNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "ParametricEqNode",
      //  "Parametric Equalizer",
      //  "asd",
      //  "Filters",
      //  "Very bad EQ",
      //  [](NodeInfo* info) { return new ParametricEqNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "SimpleComressorNode",
      //  "Simple Compressor",
      //  "asd",
      //  "Dynamics",
      //  "Description",
      //  [](NodeInfo* info) { return new SimpleComressorNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "SimpleGateNode",
      //  "Simple Gate",
      //  "asd",
      //  "Dynamics",
      //  "Description",
      //  [](NodeInfo* info) { return new SimpleGateNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "AutoGainNode",
      //  "Auto Linear Gain",
      //  "asd",
      //  "Dynamics",
      //  "Description",
      //  [](NodeInfo* info) { return new AutoGainNode(info); }
      //  });
