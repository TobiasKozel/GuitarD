import("stdfaust.lib");
gain = hslider("gain",0.5, 0, 1,0.01);
// gain = 0.5;
process = _ * gain, _ * gain;