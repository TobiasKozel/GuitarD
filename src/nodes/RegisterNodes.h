#pragma once
#include "../misc/NodeList.h"
#include "./simple_drive/SimpleDriveNode.h"
#include "./bitcrusher/BitCrusherNode.h"
#include "./rectify/RectifyNode.h"

//#include "./stereo_tool/StereoToolNode.h"
// #include "./simple_delay/SimpleDelayNode.h"
//#include "./simple_cab/SimpleCabNode.h"
//#include "./crybaby/CryBabyNode.h"
//#include "./combine/CombineNode.h"
//#include "./feedback/FeedbackNode.h"
//#include "./simple_reverb/SimpleReverbNode.h"
//#include "./simple_gate/SimpleGateNode.h"
//#include "./simple_compressor/SimpleComressorNode.h"
//#include "./fuzz/FuzzNode.h"

//#include "./overdrive/OverDriveNode.h"
//#include "./paramerticeq/ParametricEqNode.h"
//#include "./phasetool/PhaseToolNode.h"
//#include "./envelope/EnvelopeNode.h"
//#include "./autogain/AutoGainNode.h"
//#include "./powersag/PowerSagNode.h"
//#include "./reverse_delay/ReverseDelayNode.h"
//#include "./cab_lib/CabLibNode.h"
//#include "./split/SplitNode.h"
//#include "./band_split/BandSplitNode.h"
//#include "./flanger/FlangerNode.h"
//#include "./autowah/AutoWahNode.h"
//#include "./phaser/PhaseNode.h"
//#include "./lfo/LfoNode.h"
//#include "./rectify/RectifyNode.h"
//#include "./transpose/TransposeNode.h"

//registerNode(NodeInfo{
//  "SimpleDelayNode",
//  "Basic Delay",
//  "asd",
//  "Delays/Reverbs",
//  "A very simple delay effect",
//  [](NodeInfo* info) { return new SimpleDelayNode(info); }
//  });
    //RegisterProxy<SimpleDriveNode> registerSimpleDriveNode({
    //  "SimpleDriveNode", "Soft Clipper",
    //  "asd", "Distortion", "Description"
    //});

      //registerNode(NodeInfo{
      //  "SimpleDriveNode",
      //  "Soft Clipper",
      //  "asd",
      //  "Distortion",
      //  "Description",
      //  [](NodeInfo* info) { return new SimpleDriveNode(info); }
      //  });
      //registerNode(NodeInfo{
      //  "CombineNode",
      //  "Combine",
      //  "asd",
      //  "Signal Flow",
      //  "Will combine two signals",
      //  [](NodeInfo* info) { return new CombineNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "SplitNode",
      //  "Split L/R",
      //  "asd",
      //  "Signal Flow",
      //  "Splits a signal into Left/Right and Mid/Side channels",
      //  [](NodeInfo* info) { return new SplitNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "BandSplitNode",
      //  "Band Split",
      //  "asd",
      //  "Signal Flow",
      //  "Splits up a signal into three frequency bands with Butterworth High/Lowpass filters",
      //  [](NodeInfo* info) { return new BandSplitNode(info); }
      //  });


      //registerNode(NodeInfo{
      //  "StereoToolNode",
      //  "Stereo Tool",
      //  "asd",
      //  "Tools",
      //  "Does panning and Stereo width",
      //  [](NodeInfo* info) { return new StereoToolNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "FeedbackNode",
      //  "Feedback",
      //  "asd",
      //  "Signal Flow",
      //  "Allows feeding back an output signal into an input socket (Unstable)",
      //  [](NodeInfo* info) { return new FeedbackNode(info); }
      //  });

      //registerNode(NodeInfo{
      //  "PhaseToolNode",
      //  "Phase Tool",
      //  "asd",
      //  "Tools",
      //  "Delays a signal",
      //  [](NodeInfo* info) { return new PhaseToolNode(info); }
      //  });

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

      //registerNode(NodeInfo{
      //  "SimpleDriveNode",
      //  "Soft Clipper",
      //  "asd",
      //  "Distortion",
      //  "Description",
      //  [](NodeInfo* info) { return new SimpleDriveNode(info); }
      //  });

      //registerNode(NodeInfo{
      //    "OverDriveNode",
      //    "Overdrive",
      //    "asd",
      //    "Distortion",
      //    "Description",
      //  [](NodeInfo* info) { return new OverDriveNode(info); }
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
      //  "PowerSagNode",
      //  "Power Sag",
      //  "asd",
      //  "Distortion",
      //  "Emulates amp sag effect",
      //  [](NodeInfo* info) { return new PowerSagNode(info); }
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
      //  "SimpleReverbNode",
      //  "Stereo Reverb",
      //  "asd",
      //  "Delays/Reverbs",
      //  "Zika Reverb",
      //  [](NodeInfo* info) { return new SimpleReverbNode(info); }
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
      //  "CryBabyNode",
      //  "Crybaby",
      //  "asd",
      //  "Filters",
      //  "Description",
      //  [](NodeInfo* info) { return new CryBabyNode(info); }
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
