import("stdfaust.lib");

level = vslider("Wah",1, 0, 1, 0.001);

process = sp.stereoize(ve.autowah(level));