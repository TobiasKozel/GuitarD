import("stdfaust.lib");

thresh = vslider( "Threshold", -120, -120, 0, 0.1);
hold = vslider( "Hold", 0.1, 0.01, 1, 0.001);
att = vslider( "Attack", 0.01, 0.001, 1, 0.001);
rel = vslider( "Release", 0.1, 0.01, 1, 0.001);

process = ef.gate_stereo(thresh, att, hold, rel);