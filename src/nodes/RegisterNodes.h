#pragma once
#include "../misc/NodeList.h"

/**
 *      All nodes should be included in here to avoid any
 *      problems with this header only stuff
 */

/**
 * Dynamics
 */
#include "./simple_gate/SimpleGateNode.h"
#include "./simple_compressor/SimpleComressorNode.h"
#include "./autogain/AutoGainNode.h"

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
#include "./reverse_delay/ReverseDelayNode.h"

/**
 * Filters
 */
#include "./crybaby/CryBabyNode.h"
#include "./autowah/AutoWahNode.h"
#include "./transpose/TransposeNode.h"
#include "./phaser/PhaseNode.h"
#include "./paramerticeq/ParametricEqNode.h"
#include "./flanger/FlangerNode.h"

/**
 * Routing
 */
#include "./combine/CombineNode.h"
#include "./feedback/FeedbackNode.h"
#include "./split/SplitNode.h"
#include "./band_split/BandSplitNode.h"

/**
 * Cab stuff
 */
#include "./simple_cab/SimpleCabNode.h"


//#include "./envelope/EnvelopeNode.h"
//#include "./cab_lib/CabLibNode.h"
//#include "./lfo/LfoNode.h"

//registerNode(NodeInfo{
      //registerNode(NodeInfo{
      //  "EnvelopeNode",
      //  "Envelope Automation Tool",
      //  "asd",
      //  "Automation",
      //  "Description",
      //  [](NodeInfo* info) { return new EnvelopeNode(info); }
      //});

      //registerNode(NodeInfo{
      //  "LfoNode",
      //  "LFO Automation Tool",
      //  "asd",
      //  "Automation",
      //  "Description",
      //  [](NodeInfo* info) { return new LfoNode(info); }
      //});

      //registerNode(NodeInfo{
      //  "SimpleCabNode",
      //  "Simple Cabinet",
      //  "asd",
      //  "Cabinets",
      //  "Provides a few IRs and allow loading .wav IRs up to 10 seconds",
      //  [](NodeInfo* info) { return new SimpleCabNode(info); }
      //});

      //registerNode(NodeInfo{
      //  "CabLibNode",
      //  "Cabinet Library",
      //  "asd",
      //  "Cabinets",
      //  "Allows flipping through IRs quickly. Offline and Online (Soundwoofer) IR database",
      //  [](NodeInfo* info) { return new CabLibNode(info); }
      //});
