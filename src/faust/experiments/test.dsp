import("stdfaust.lib");
import("reverbs.lib");
gain = hslider("gain",0.5,0,1,0.01);
// process = _,_ : zita_rev1_stereo(0,500,4000,10,10,48000) : _ * gain,_ * gain;

process = 
    dm.cubicnl_demo <: // distortion 
//    dm.wah4_demo <: // wah pedal
    dm.phaser2_demo : // stereo phaser 
    dm.compressor_demo : // stereo compressor
    dm.zita_light; // stereo reverb
