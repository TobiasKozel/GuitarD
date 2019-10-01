import("stdfaust.lib");
import("reverbs.lib");
gain = hslider("gain",0.5,0,1,0.01);
process = _,_ : zita_rev1_stereo(0,500,4000,10,10,48000) : _ * gain,_ * gain;
