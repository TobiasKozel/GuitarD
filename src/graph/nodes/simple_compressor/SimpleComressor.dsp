import("stdfaust.lib");

ratio = vslider( "Ratio", 1, 0.1, 10, 0.01);
thresh = vslider( "Threshold", 0, -70, 0, 0.1);
att = vslider( "Attack", 0.01, 0.001, 0.1, 0.001);
rel = vslider( "Release", 0.1, 0.01, 1, 0.001);

process = _,_ : co.compressor_stereo(ratio,thresh,att,rel) : _,_;